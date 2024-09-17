import { useEffect, useState } from 'react';
import './App.css'
import statusbar from './assets/statusbar.png';
import contact from './assets/contact.png';

/*
async function readData(server) {
  console.log('retrieving data...');
  let service = await server.getPrimaryService('0000ffe0-0000-1000-8000-00805f9b34fb');
  let ble = await service.getCharacteristic('0000ffe1-0000-1000-8000-00805f9b34fb');
  let dataRead = await ble.readValue();
  let dataValue = dataRead.getUint8(0);
  console.log(dataValue);
  return dataValue;
}
*/

async function writeData(server, value) {
  console.log('writing data...');
  let service = await server.getPrimaryService('0000ffe0-0000-1000-8000-00805f9b34fb');
  let ble = await service.getCharacteristic('0000ffe1-0000-1000-8000-00805f9b34fb');
  let commandValue = new Uint8Array([value]);
  await ble.writeValue(commandValue);
}

const readStatuses = {
  1: 'Walking Safely ✅',
  2: 'Fall ❗️',
  3: 'Not Connected',
}

const writeStatuses = {
  'connected': 0x00,
  'disconnected': 0x01,
}

function App() {

async function setupBluetooth() {
  let device = await navigator.bluetooth.requestDevice({
    filters: [{name: 'BT05'}],
    optionalServices: ['0000ffe0-0000-1000-8000-00805f9b34fb'],
   })
  let server = await device.gatt?.connect();
  writeData(server, writeStatuses.connected);
  let service = await server.getPrimaryService('0000ffe0-0000-1000-8000-00805f9b34fb');
  let ble = await service.getCharacteristic('0000ffe1-0000-1000-8000-00805f9b34fb');

  // eslint-disable-next-line no-unused-vars
  ble.startNotifications().then(_ => {
    console.log('Notifications started');
    ble.addEventListener('characteristicvaluechanged',
      (e) => {
        let data = e.target.value.getUint8(0)
        console.log("Received:", data);
        setStatus(data);
      });
   });
   
  return server, device;
}

  const [server, setServer] = useState(null);
  const [status, setStatus] = useState(3);

  const override = false; // set to false after done generating screenshots

  useEffect(() => {
    if (status == 2) {
      alert('FALL DETECTED!');
    }
  }, [status])

  return (
    <>
      <div id="fullWrapper" style={{display: "flex", flexDirection: "column"}}>
      <img src={statusbar}></img>
      <div id="deviceTextWrapper">
        <h1>Your Device</h1>
        <h2>{"Ding Yi's Smart Cane"}</h2>
      </div>
      <div id="statusCardWrapper">
        <button onClick={async () => {
          let server = await setupBluetooth((e) => console.log(e));
          setServer(server);
        }}>{!server && !override ? "Pair device" : "Connected ✅"}
        </button>
        <h4 style={{color: status === 1 ? "green" : status === 2 ? "red" : "grey"} }>
          {readStatuses[status]}
        </h4>
      </div>
      <div id="emergencyContactTextWrapper">
        <h1>Emergency Contact</h1>
        <img src={contact}></img>
      </div>
      <div id="emergencyContactCardsWrapper">
        <div id="emergencyContactCard">

        </div>
      </div>
      </div>
    </>
  )
}

export default App
