//
//  ViewController.swift
//  ArduinoPlayground
//
//  Created by Boris Erceg on 28.10.2014..
//  Copyright (c) 2014. @kviksilver. All rights reserved.
//

import Cocoa



class ViewController: NSViewController {

    @IBOutlet var serialPortSelector: NSPopUpButton!
    @IBOutlet var arrayController: NSArrayController!
    @IBOutlet var selectedObject: ORSSerialPort!
    
    override func viewDidLoad() {
        super.viewDidLoad()
        arrayController.content = ORSSerialPortManager.sharedSerialPortManager().availablePorts
    }

}
