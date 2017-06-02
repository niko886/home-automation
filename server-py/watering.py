#!/usr/bin/python
# -*- coding: utf8 -*-


import threading
import paho.mqtt.client as mqtt
import paho.mqtt.publish as publish
import time

import conf

from datetime import datetime
from optparse import OptionParser

_VERSION_ = '0.0.3'

class myThread (threading.Thread):
    def __init__(self, event):
        threading.Thread.__init__(self)
        self._event = event        
        self._vc = valveControl()
      
    def run(self):
        
        self._vc.makeFun(self._event)

def on_connect(client, userdata, flags, rc):
    
    print('Connected with result code '+str(rc))

    client.subscribe('water/control')    
    client._event = threading.Event()    
    client._mythreads = []
    client._vc = valveControl()

def on_message(client, userdata, msg):
    
    print(msg.topic+' '+str(msg.payload))
    
    if msg.payload == 'allOn':

        client._event.clear()                    
        for t in client._mythreads:
            t.join()             
        client._mythreads = []

        client._vc.turnAll(1)

    if msg.payload == 'makeFun':
        
        client._event.set()
        
        t = myThread(client._event)
        client._mythreads.append(t)
        t.start()

    if msg.payload == 'allOff':
        
        client._event.clear()
                
        for t in client._mythreads:
            t.join()             
        client._mythreads = []
        
        client._vc.turnAll(0)

class valveControl():
    
    _zones = ['0', '1', '3']
    _dropZones = ['2']
    _event = None

    def __init__(self):
        
        pass

    def turnAll(self, onOff, dropOnly=False, wateringOnly=False):
        
        
        zones = []
        
        
        if dropOnly:            
            zones += self._dropZones
            
        if wateringOnly:
            zones += self._zones
            
        if not dropOnly and not wateringOnly:
            zones = self._zones + self._dropZones
            
            
        messages = []
        
        for i in zones:
            
            oneMessage = {'topic': 'water',
                          'payload': '%s%s' % (str(i), str(onOff))}
            
            messages.append(oneMessage)
            
        print('[*] publishing: %s' % messages)
        publish.multiple(messages, hostname=conf.SERVER_ADDR)
            
        
    def dropOn(self):
        
        
        messages = []
        
        for i in self._dropZones:
            
            oneMessage = {'topic': 'water',
                          'payload': '%s%s' % (str(i), '1')}
            
            messages.append(oneMessage)        

        print('[*] publishing: %s' % messages)
        publish.multiple(messages, hostname=conf.SERVER_ADDR)
            
        
    def round1(self, delay):
        
        for z in self._zones:            
            
            if self._event and not self._event.is_set():
                return
            
            publish.single('water', z + '1', hostname=conf.SERVER_ADDR)
            print('water', z + '1')
            
            time.sleep(delay)
            
            publish.single('water', z + '0', hostname=conf.SERVER_ADDR)
            print('water', z + '0')

    def round2(self, delay):

        if self._event and not self._event.is_set():
            return
    
        self.turnAll(1)
        time.sleep(delay)
        self.turnAll(0)
        time.sleep(delay)
    
    def round3(client, delay):

        if self._event and not self._event.is_set():
            return
        
        turnAll(1)
        time.sleep(delay)
        turnAll(0)
    

    def makeFun(self, event):
    
        print('making some fun...')
        
        self.turnAll(0)
        self._event = event
    
        while event.is_set():
        
            for i in range(0, 3):
                self.round1(1)
                
                
            if not event.is_set():
                break
     
            for i in range(0, 3):
                self.round2(0.5)
    
            if not event.is_set():
                break
        
            for i in range(0, 5):
                self.round1(0.5)
    
            if not event.is_set():
                break
               
            self.round3(5)
    
        print('fun done')        
    
if __name__ == '__main__':
        
    vc = valveControl()    
    
    parser = OptionParser(usage='usage: %prog [options]', version='%prog ' + _VERSION_)    
    
    parser.add_option('-1', '--on',
                  action='store_true', dest='on', default=False,
                  )

    parser.add_option('-r', '--drop-on',
                  action='store_true', dest='dropOn', default=False,
                  )

    parser.add_option('-w', '--watering-on',
                  action='store_true', dest='wateringOn', default=False,
                  )
    
    parser.add_option('-0', '--off',
                  action='store_true', dest='off', default=False,
                  )

    parser.add_option('-f', '--fun',
                  action='store_true', dest='fun', default=False,
                  )

    parser.add_option('-d', '--daemon',
                  action='store_true', dest='daemon', default=False,
                  )

    parser.add_option('-t', '--print-time',
                  action='store_true', dest='printTime', default=False,
                  )

    (options, args) = parser.parse_args()


    if options.printTime:
        print(datetime.strftime(datetime.now(), '%Y-%m-%d %H:%M:%S'))
    
    if options.on:
        vc.turnAll(1)

    elif options.off:
        vc.turnAll(0)

    elif options.fun:
        vc.makeFun()
        
    elif options.dropOn:
        vc.dropOn()
        
    elif options.wateringOn:
        vc.turnAll(1, wateringOnly=True)
        
    elif options.daemon:
        
        cli = mqtt.Client()
        cli.on_connect = on_connect
        cli.on_message = on_message
          
        cli.connect(conf.SERVER_ADDR, 1883, 60)    
            
        cli.loop_forever()
    
    