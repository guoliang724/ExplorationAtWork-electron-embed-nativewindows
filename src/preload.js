const { contextBridge, ipcRenderer } = require('electron');
const addon = require('../build/Release/addon');
contextBridge.exposeInMainWorld('electronApi', {
  addon,
  openDialog: () => ipcRenderer.invoke('open:dialog'),
  openFile: (filePath) => ipcRenderer.invoke('open:file', filePath),
  getArray: (cb) => ipcRenderer.on('arry', cb)
});
