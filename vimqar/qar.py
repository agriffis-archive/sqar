#!/usr/local/bin/python
import telnetlib

class QAR:
    def __init__(self, host='gorge', user='DUSTIN', db='OSF_QAR', maint='CHOUSER'):
        self.maint = maint

        # connect
        print 'connecting...'
        self.tn=telnetlib.Telnet('gorge')

        # log in for osf_qar
        self.tn.read_until('Username:')
        print 'logging in...'
        self.writeln(user)
        self.tn.read_until('Which QAR database?')
        print 'choosing database...'
        self.writeln(db)
        self.tn.read_until('QAR>')

    def writeln(self, line=''):
        self.tn.write(line + '\r\n')

    def dir(self, maint=None):
        if not maint:
            maint = self.maint

        # get a directory of QARs
        print 'getting dir...'
        self.writeln('DIR MAINT ' + maint)
        print self.getresp()

    def getresp(self):
        fulltext = ''
        while 1:
            (index, match, text) = self.tn.expect([
                    'QAR>', 
                    '(.*)\[(.*)\]\?', 
                    'Hit return for next page.*:'
                ], 3)
            fulltext += text
            if index == 0:
                return fulltext
            elif index == 1:
                cmd = raw_input('Prompt: ')
                self.writeln(cmd)
            elif index == 2:
                self.writeln()
            else:
                print '--TIMEOUT ERROR--'
                print fulltext
                return fulltext

q = QAR()
q.dir()
while 1:
    cmd = raw_input('QAR command: ')
    q.writeln(cmd)
    print q.getresp()
