console.log('addon:', window.electronApi);
const btn = document.querySelector('.openFile');
const sp = document.querySelector('.pathName');
const router1 = document.querySelector('.router');

console.log('Main Page: :');

const objHandle = {
  exeHandle: 0
};

btn.addEventListener('click', async () => {
  const filePath = await window.electronApi.openDialog();
  sp.innerHTML = filePath;
  const result = await window.electronApi.openFile(filePath);
});

window.electronApi.getArray((event, arg) => {
  console.log('updated-windows', arg);
  if (arg.length !== 0) objHandle.exeHandle = arg[0].handle;
});

// router1.addEventListener('click', () => {
//   const query = location.search;
//   console.log('main Page: ', query);
//   if (query) {
//     const arg = query.substring(1);
//     window.location.href = `./a.html?handle=${arg}`;
//     // window.electronApi.addon.showWindow(arg, 'hide');
//   } else if (objHandle.exeHandle) {
//     window.location.href = `./a.html?handle=${objHandle.exeHandle}`;
//     //window.electronApi.addon.showWindow(objHandle.exeHandle, 'hide');
//   } else {
//     alert('先启动应用');
//     window.location.href = `./a.html`;
//   }
// });
