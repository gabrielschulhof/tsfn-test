const { Worker, isMainThread, workerData, parentPort } = require('node:worker_threads');
const process = require('node:process');
if (isMainThread) {
  const threadOptions = {
    bindingName: 'tsfn_test' + (process.argv.slice(2).includes('c') ? '_c': '') + '.node',
    runForever: process.argv.slice(2).includes('runForever'),
  };
  console.log(threadOptions);
  const worker = new Worker(__filename, {
    workerData: threadOptions,
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
  })(workerData.bindingName));
  function sync_write(str) {
    for (let result = process.stdout.write(str + '\n');
         !result;
         result = process.stdout.write(''));
  }
  const x = new tsfn_test((value) => {
    sync_write('JS: Called with ' + value);
  });
  if (!workerData.runForever) {
    parentPort.postMessage('terminate me');
  }
}
