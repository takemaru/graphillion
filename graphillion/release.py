# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
# 
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
# OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
# WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

"""Release data for Graphillion.
"""

authors = []

# read the version and authors from pyproject.toml file
with open('pyproject.toml') as f:
    import re
    pattern = r'\{name\s*=\s*"(.*?)"(?:,\s*email\s*=\s*"(.*?)")?\},?'

    author_mode = False
    for line in f:
        if line.startswith('version'):
            a = line.split('=')
            version = a[1].strip().replace('"', '')
        elif line.startswith('authors'):
            author_mode = True
        elif author_mode:
            if line.startswith(']'):
                author_mode = False
            else:
                # parse '{name = "xxx", email = "yyy"}'
                m = re.match(pattern, line.strip())
                if m:
                    if m.group(2):
                        email = m.group(2)
                    else:
                        email = ""
                    authors.append((m.group(1), email))

authors = tuple(authors)

#version = '2.0rc0'
date = ''
license = 'MIT'
#authors = ()
