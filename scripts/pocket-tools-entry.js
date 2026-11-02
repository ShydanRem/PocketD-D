// Shared PC operations bundled for the extension. Port state stays in a single
// external module shared with account linking; no account secrets enter here.
export {checkDeviceConnection,readDeviceDiagnostics,receiveDeviceBackup,restoreDeviceBackup} from '../src/device-usb.js';
export {prepareSdAssets,installSdAssets} from '../src/sd-assets.js';
