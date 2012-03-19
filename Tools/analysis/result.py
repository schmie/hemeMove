#!/usr/bin/env python
# encoding: utf-8
"""
result.py

Created by James Hetherington on 2012-01-23.
Copyright (c) 2012 __MyCompanyName__. All rights reserved.
"""

import os
import re
import yaml
import datetime
import functools
import subprocess
from xml.etree import ElementTree
from hemeTools.parsers.geometry.simple import ConfigLoader

import logging
import environment
logger=logging.getLogger('parsing')

class FileModel(object):
    def __init__(self,relative_path,loader):
        self.loader=loader
        self.path=relative_path
        self.key=relative_path+loader.__name__
    def fullpath(self,result):
        return os.path.expanduser(os.path.join(result.path,self.path))
    def model(self,result):
        if result.files.get(self.key): return result.files.get(self.key)
        result.files[self.key]=result.files.get(self.key) or self.loader(self.fullpath(result))
        self.logger(result).debug("Loaded")
        return result.files[self.key]
    def logger(self,result):
        return logging.LoggerAdapter(logger,dict(context=self.fullpath(result)))

class ResultContent(object):
    def __init__(self,filter):
        self.filter=filter
    def model(self,result):
        return self.filter(result)
    def logger(self,result):
        return logging.LoggerAdapter(logger,dict(context=result.path))

class ResultProperty(object):
    def __init__(self,label,memoized_file_model,parser,pattern):
        self.pattern=pattern
        self.file=memoized_file_model
        self.label=label
        self.parser=parser
    @staticmethod
    def parse_value(value):
        if value in ['None','none',None]:
            return None
        if value in ['True','true',True]:
            return True
        if value in ['False','false',False]:
            return False
        try:
            return int(value)
        except (TypeError,ValueError):
            try:
                return float(value)
            except (TypeError,ValueError):
                try:
                    return float(value.replace("_","."))
                except (TypeError,ValueError):
                    try:
                        return value.strip()
                    except AttributeError:
                        return value
    def get(self,result):
        try:
            model=self.file.model(result)
            if not model:
                raise ParseError("Bad file.")
            value=self.parser(model,self.pattern)
            result.properties[self.label]=result.properties.get(self.label) or self.parse_value(str(value))
            return result.properties.get(self.label)
        except (IOError,ParseError, OSError) as err:
            self.file.logger(result).warning("Problem parsing value: %s"%err)
            return None
    # This defines how, when an instance of this class is a property in a parent object, a value is obtained for it.
    def __get__(self,instance,owner):
        return self.get(instance)

class ParseError(Exception):
    def __init__(self, value):
        self.value = value
    def __str__(self):
        return repr(self.value)

def index_parser(content,pattern):
    return content.get(pattern)
def regex_parser(content,pattern):
    match=re.search(pattern,content)
    if not match: return None
    return re.search(pattern,content).groups()[0]
def element_parser(content,pattern):
    attribute=None
    if type(pattern)==list:
        # we have a two-tuple in the yaml, the second argument is an attribute name for the element
        pattern,attribute=pattern
    element=content.find(pattern)
    if element==None:
        logger.error
        raise ParseError("No element %s"%pattern)
    if attribute:
        try:
            return element.get(attribute)
        except AttributeError:
            raise ParseError("No attribute %s on element %s"%attribute,pattern)
    else:
        return element.text
def identity_parser(content,pattern):
    return pattern
def eval_parser(content,pattern):
    try:
        # Since the properties are dynamic, they aren't in vars(self), so we have to build the binding.
        # Bind only the expressions in the pattern
        # The "content" supplied must respond to () to produce the binding
        return eval(pattern,globals(),content(pattern))
    except Exception as err:
        raise ParseError("Problem handling expression %s: %s"%(pattern,err))
def attribute_parser(content,pattern):
    return getattr(content,pattern)
def fncall_parser(content,pattern):
    out= content(pattern)
    return out

def yaml_loader(path):
    return yaml.load(open(path))
def text_loader(path):
    return open(path).read()
def xml_loader(path):
    try:
        return ElementTree.parse(path)
    except ElementTree.ParseError:
        raise ParseError("Could not parse file.")
def stat_loader(path):
    return os.stat(path)
def geometry_header_loader(path):
    model=ConfigLoader(path)
    model._LoadPreamble()
    model._LoadHeader()
    def binder(expression):
        return model.Domain.__dict__
    return binder

def null_filter(result):
    return None
def name_filter(result):
    return result.name
def binding_filter(result):
    # Return a binding suitable for use in eval, from the result
    # The object so returned must respond to () to generate the binding for an expression to be evaluated
    def binder(expression):
        terms=re.split("\W",expression)
        bindings_needed=set(terms).intersection(result.proplist)
        binding={key: getattr(result,key) for key in bindings_needed}
        return binding
    return binder
def shell_filter(result):
    return functools.partial(subprocess.check_output,cwd=os.path.expanduser(result.path))
def mercurial_filter(result):
    def generator(template):
        if not result.changeset: raise ParseError("No mercurial revision specified.")
        if result.changeset[-1]=='+':
            changeset=result.changeset[:-1]
        else:
            changeset=result.changeset
        try:
            return subprocess.check_output(["hg","log","-r",changeset,"--template",template],cwd=environment.localroot)
        except subprocess.CalledProcessError as err:
            raise ParseError("Problem calling mercurial: %s"%err)
    return generator

def result_model(config):
    class Result(object):
        """Model of a result"""
        proplist=[]
        @classmethod
        def define_file_properties(klass,config,loader,parser):
            if not config: return
            for path,data in config.iteritems():
                klass.define_properties(FileModel(path,loader),data,parser)

        @classmethod
        def define_properties(klass,file_model,data,parser):
            if not data: return
            for prop,pattern in data.iteritems():
                klass.proplist.append(prop)
                setattr(klass,prop,ResultProperty(prop,file_model,parser,pattern))

        def __init__(self,path):
            """path: the path to the result folder
               config: a dictionary specifying what aspects of the result folder to make into properties of the result
            """
            self.path=os.path.expanduser(path)
            self.name=os.path.basename(self.path)
            self.properties={key:None for key in self.proplist}
            self.files={}

        def datum(self,property):
            """Return a property. If it is an unknown property, assume it is an anonymous compound property which wasn't stated beforehand."""
            if property in self.proplist:
                return getattr(self,property)
            return ResultProperty(property,ResultContent(binding_filter),eval_parser,property).get(self)

        def __str__(self):
            propstring=', '.join(["%s : %s"%(prop,self.datum(prop)) for prop in self.properties])
            return "Result %s: [%s]"%(self.name,propstring)
            
        def hash(self):
            return {prop:self.datum(prop) for prop in self.properties}
            
        def query(self,property,value):
            prop=self.datum(property)
            try:
                if value[0]=='<':
                    return prop<type(prop)(value[1:])
                elif value[0]=='>':
                    return prop>type(prop)(value[1:])
                else:
                    return prop==value
            except TypeError:
                return prop==value


    Result.define_file_properties(config.get('yaml_files'),yaml_loader,index_parser)
    Result.define_file_properties(config.get('text_files'),text_loader,regex_parser)
    Result.define_file_properties(config.get('xml_files'),xml_loader,element_parser)
    Result.define_properties(ResultContent(name_filter),config.get('name_properties'),regex_parser)
    Result.define_properties(ResultContent(null_filter),config.get('fixed_properties'),identity_parser)
    Result.define_properties(ResultContent(binding_filter),config.get('compound_properties'),eval_parser)
    Result.define_file_properties(config.get('stat_properties'),stat_loader,attribute_parser)
    Result.define_properties(ResultContent(shell_filter),config.get('shell_properties'),fncall_parser)
    Result.define_properties(ResultContent(mercurial_filter),config.get('mercurial_properties'),fncall_parser)
    Result.define_file_properties(config.get('gmy_files'),geometry_header_loader,eval_parser)
    return Result
