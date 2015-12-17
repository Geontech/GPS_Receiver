#!/usr/bin/python

# GPS Proof of Life (gps_receiver_test)
from ossie.utils import redhawk
from redhawk.frontendInterfaces import FRONTEND

# For Console control
import curses
import curses.ascii
import locale
import time

try:    
    # Setup Curses
    # locale.setlocale(locale.LC_ALL,'')
    # code = locale.getpreferredencoding()
    stdscr = curses.initscr()
    curses.noecho()
    curses.cbreak()
    stdscr.nodelay(True)
    stdscr.keypad(True)

    # Get Domain and Node
    domain = redhawk.attach("REDHAWK_DEV")
    gps_receiver = domain.devMgrs[0].devs[0]
    
    if ('gps_receiver' == gps_receiver.name):
        gps_in =  gps_receiver.getPort("GPS_in")._narrow(FRONTEND.GPS)

        if (None == gps_in):
            stdscr.addstr(0,0, "Unable to obtain GPS_in port on gps_receiver")
            stdscr.refresh()
         
        else:
            # Print samples until finished.
            stdscr.addstr(0,0, "Press enter key to stop.\n")
            update = 0;
            while True:
                gps_time_pos = gps_in._get_gps_time_pos()
                stdscr.addstr(1,0,"{0:4d} - UTC Time: {1:5.3f} (twsec)\n".format(update, gps_time_pos.timestamp.twsec))
                update += 1
                stdscr.refresh()
                
                c = stdscr.getch()
                if (c == curses.ascii.NL):
                    break;
                else:
                    time.sleep(1)
                    
    else:
        stdscr.addstr(0,0,"Failed to find gps receiver on domain")
        stdscr.refresh()
    
finally:
    # Cleanup Curses
    stdscr.keypad(False)
    stdscr.nodelay(False)
    stdscr.addstr(0,0,"Press any key to return to console.\n")
    stdscr.getch()
    curses.nocbreak()
    curses.echo()
    curses.endwin()
