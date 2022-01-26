import router from '@ohos.router'

export default {
    data: {
        pincode: router.getParams().pinCode,
    },
    onInit() {
        console.info('getParams: ' + router.getParams());
    },
    onConfirm() {
        console.info('click confirm');
        callNativeHandler("EVENT_CONFIRM", "1");
    }
}