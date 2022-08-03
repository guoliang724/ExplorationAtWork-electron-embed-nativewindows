const { app, BrowserWindow, ipcMain, dialog } = require('electron');
const addon = require('bindings')('../build/Release/addon.node');
const path = require('path');
const { execFile } = require('child_process');
const command = require('./command'); // command for exe

app.disableHardwareAcceleration();
app.commandLine.appendSwitch('force_high_performance_gpu');

const objConfig = {
  exeFilePath: null,
  exeFilaName: '13170889099的视频会议', // 这个可以通过启动参数来改
  electronPath: null,
  electronTitle: 'hellow world',
  allWindows: [],
  mainWindow: null
};

const mainWindowPostion = {
  x: 0,
  y: 0,
  width: 0,
  height: 0
};

const createWindow = () => {
  const mainWindow = new BrowserWindow({
    width: 1000,
    height: 1000,
    title: objConfig.electronTitle,
    webPreferences: {
      nodeIntegration: true,
      contextIsolation: true,
      enableRemoteModule: true,
      preload: path.join(__dirname, 'preload.js')
    }
  });
  setWindowBounds(mainWindow, 1000, 1000, 400, 400);

  // and load the index.html of the app.
  mainWindow.loadFile('index.html');

  // Open the DevTools.
  mainWindow.webContents.openDevTools();

  objConfig.mainWindow = mainWindow;
};

const handleDialog = async () => {
  const { canceled, filePaths } = await dialog.showOpenDialog({
    filters: [{ name: 'exe file', extensions: ['exe'] }]
  });
  if (canceled) return;
  else return filePaths[0];
};

app.whenReady().then(() => {
  // listening for diaolog's opening
  ipcMain.handle('open:dialog', handleDialog);

  //listen to receive the filePath
  ipcMain.handle('open:file', async (event, arg) => {
    setPathAndTitle(arg);

    // get electron info
    getElectronInfo();

    // run application
    await runExe();

    // retrieve all handles we need
    const result = await getAllWindows();

    if (result.code == 1) {
      //const postion = addon.getWindowBounds(result.electronHandle);

      // set the postion(目前无效)
      await addon.setWindowBounds(result.exeHandle, mainWindowPostion);

      // merge windows
      await mergeWindow(result);
    }
  });

  createWindow();
});

app.on('window-all-closed', function () {
  if (process.platform !== 'darwin') app.quit();
});

function sleep(ms) {
  return new Promise((resolve) => setTimeout(resolve, ms));
}

async function getAllWindows() {
  let allWindows = [];
  let exeWindow = [];
  let count = 0;

  do {
    if (count === 35)
      throw new Error(Buffer.from('启动超时', 'utf-8').toString());
    allWindows = await addon.getWindows();
    await sleep(1000);
    exeWindow = allWindows.filter((i) => i.title == objConfig.exeFilaName);
    objConfig.mainWindow.webContents.send('arry', exeWindow);
    count++;
  } while (exeWindow.length === 0);

  const eleWindow = allWindows.filter(
    (i) =>
      i.path == path.normalize(objConfig.electronPath) &&
      i.title == objConfig.electronTitle
  );
  if (eleWindow.length !== 0 && exeWindow.length !== 0) {
    const electronHandle = eleWindow[0].handle;
    const exeHandle = exeWindow[0].handle;

    const { d3dChild, contentChild } = findChildWindow(electronHandle);

    return { code: 1, exeHandle, electronHandle, d3dChild, contentChild };
  }

  return { code: 0 };
}

async function runExe() {
  try {
    const { stdout, stderr } = await execFile(objConfig.exeFilePath, command, {
      windowsHide: true
    });
  } catch (e) {
    console.error(e); // should contain code (exit code) and signal (that caused the termination).
  }
}

function setPathAndTitle(arg) {
  objConfig.exeTitle = path.basename(arg);
  objConfig.exeFilePath = arg.replaceAll('\\', '\\\\');
}

function getElectronInfo() {
  temp = process.cwd() + `\\node_modules\\electron\\dist\\electron.exe`;
  objConfig.electronPath = temp.replaceAll('\\', '\\\\');
}

async function mergeWindow({
  electronHandle,
  d3dChild,
  contentChild,
  exeHandle
}) {
  await addon.setParent(electronHandle, d3dChild, contentChild, exeHandle);
}

function findChildWindow(parentWindowhandle) {
  const childWindows = addon.getChildWindows(parentWindowhandle);
  console.log('childWindos', childWindows);
  let d3dChild, contentChild;
  if (childWindows.length > 1) {
    d3dChild = childWindows[0].handle;
    contentChild = childWindows[1].handle;
  } else if (childWindows.length == 1) {
    d3dChild = childWindows[0].handle;
    contentChild = childWindows[0].handle;
  } else {
    d3dChild = contentChild = 0;
  }
  return { d3dChild, contentChild };
}

// set the relative postion
function setWindowBounds(window, xMove, yMove, _width, _height) {
  const { x, y, width, height } = window.getBounds();
  mainWindowPostion.x = x + xMove;
  mainWindowPostion.y = y + yMove;
  mainWindowPostion.width = _width;
  mainWindowPostion.height = _height;
}

// const objConfig = {
//   exePath: 'C:\\Users\\user\\Desktop\\Meeting\\Meeting.exe',
//   electronPath:
//     'C:\\Users\\user\\Desktop\\test\\electron-quick-start\\node_modules\\electron\\dist\\electron.exe',
//   exeTitle: '13170889099的视频会议',
//   electronTitle: 'Hello World!'
// };
