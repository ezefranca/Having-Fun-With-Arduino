//
//  ViewController.swift
//  ArduinoPlayground
//
//  Created by Boris Erceg on 28.10.2014..
//  Copyright (c) 2014. @kviksilver. All rights reserved.
//

import Cocoa



class ViewController: NSViewController , ORSSerialPortDelegate{

    @IBOutlet var serialPortSelector: NSPopUpButton!
    @IBOutlet var arrayController: NSArrayController!
    @IBOutlet var selectedObject: ORSSerialPort!
    
    @IBOutlet var infoLabel: NSTextField!
    @IBOutlet var baudRateSelector: NSTextField!
    override func viewDidLoad() {
        super.viewDidLoad()
        arrayController.content = ORSSerialPortManager.sharedSerialPortManager().availablePorts
    }

    @IBAction func buttonPressed(sender: AnyObject) {
        
        selectedObject.delegate = self;
        if let baud = baudRateSelector.stringValue.toInt() {
            selectedObject.baudRate = baud
        }
        selectedObject.open();
        
    }
    
    func serialPort(serialPort: ORSSerialPort!, didEncounterError error: NSError!) {
        
    }
    
    func serialPort(serialPort: ORSSerialPort!, didReceiveData data: NSData!) {
        var receivedString = NSString(data: data, encoding: NSUTF8StringEncoding)
        infoLabel.stringValue = receivedString as String
    }
    
    func serialPortWasClosed(serialPort: ORSSerialPort!) {
        
    }
    
    func serialPortWasOpened(serialPort: ORSSerialPort!) {
        
    }
    
    func serialPortWasRemovedFromSystem(serialPort: ORSSerialPort!) {
        
    }
}
