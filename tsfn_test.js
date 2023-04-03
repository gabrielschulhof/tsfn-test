const { Worker, isMainThread, workerData, parentPort } = require('node:worker_threads');
const process = require('node:process');
if (isMainThread) {
  const cSuffix = (process.argv[2] === 'c' ? '_c': '');
  const bindingName = 'tsfn_test' + cSuffix + '.node';
  const worker = new Worker(__filename, {
    workerData: bindingName
  });
  worker.on('message', () => {
    setImmediate(() => {
      console.log('JS: terminating worker');
      worker.terminate();
    });
  });
} else {
  const tsfn_test = require(((bindingName) => {
    try {
      return require.resolve('./build/Release/' + bindingName);
    } catch(err) {
      return require.resolve('./build/Debug/' + bindingName);
    }
  })(workerData));
  function sync_write(str) {
    for (let result = process.stdout.write(str + '\n');
         !result;
         result = process.stdout.write(''));
  }
  const x = new tsfn_test((value) => {
    sync_write('JS: Called with ' + value);
  });
  parentPort.postMessage('terminate me');
}
