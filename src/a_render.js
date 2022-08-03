const btn = document.querySelector('.back');
const query = window.location.search.substring(1);
const addon = window.electronApi.addon;
console.log('Page A');
console.log('query', query);
console.log('pageA addon: ', window.electronApi.addon);
btn.addEventListener('click', () => {
  window.location.href = `./index.html?handle=${query}`;
  addon.showWindow(query, 'show');
});
