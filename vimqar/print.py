import vim
import thread
import time

def spin(buf, n):
    i = 0
    while 1:
        if buf[0] == "quit": break
        print "bg: %d" % i
        i+=1
        #time.sleep(1)
    print "done"

def go():
    thread.start_new_thread(spin, (vim.current.buffer, 0))
