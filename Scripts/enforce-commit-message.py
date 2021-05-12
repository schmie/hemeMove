# This file is part of HemeLB and is Copyright (C)
# the HemeLB team and/or their institutions, as detailed in the
# file AUTHORS. This software is provided under the terms of the
# license in the file LICENSE.

import re

# Use via
# [hooks]
# pretxncommit = python:/path_to_hemelb/Scripts/enforce-commit-message.py:checkCommitMessage

MAX_LINE_LENGTH = 80

def checkCommitMessage(ui, repo, **kwargs):
    """
    Checks a single commit message for adherence to commit message rules.  

    To use add the following to your project .hg/hgrc for each
    project you want to check, or to your user hgrc to apply to all projects.

    [hooks]
    pretxncommit = python:path/to/script/enforce-message.py:checkCommitMessage
    """

    message = repo['tip'].description()
    error = False

    lines = message.split('\n')
    for i, line in enumerate(lines):
      if len(line) > MAX_LINE_LENGTH:
        ui.warn('Invalid commit message: line %d had %d characters (limit is %d)\n' % (i+1, len(line), MAX_LINE_LENGTH))
        error = True

    if not re.match('\Amerge\W', lines[0], re.IGNORECASE) and not re.search('\#\d+', lines[0]):
      ui.warn('Your first line must either begin with the word "merge" or have a ticket id in it (e.g. "#345")\n')
      error = True
 
    return error

