#!/usr/bin/env python
# This file is part of HemeLB and is Copyright (C)
# the HemeLB team and/or their institutions, as detailed in the
# file AUTHORS. This software is provided under the terms of the
# license in the file LICENSE.

import os

import environment
from results_collection import ResultsCollection
from graph import Graph
from report import Report

class Analysis(object):
    def __init__(self,config):
        self.results_path = config['results_path']
        self.reports_path = config['reports_path']
        self.graph_configuration = config['graphs']
        self.report_configuration = config['reports']
        self.result_configuration = config['results']
        self.graphs = {label: Graph(data)
                       for label, data in self.graph_configuration.iteritems()}
        self.reports = {label: Report(data, self.graphs)
                        for label, data in self.report_configuration.iteritems()}
    
    def load_data(self):
        self.results = ResultsCollection(self.results_path,
                                         self.result_configuration)
        
    def prepare(self):
        for report in self.reports.itervalues():
            report.prepare(self.results)
            
    def write(self):
        for label, report in self.reports.iteritems():
            report.write(os.path.expanduser(os.path.join(self.reports_path, label)))
            print "Report %s written to %s" % (report.name, report.path)

def main():
    print 'Loading environment'
    analysis = Analysis(environment.config)
    print 'Loading data'
    analysis.load_data()
    print 'Preparing data'
    analysis.prepare()
    print 'Writing reports'
    analysis.write()

if __name__ == '__main__':
    main()

