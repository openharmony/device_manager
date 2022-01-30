import router from '@ohos.router';

var numbs = 0;
export default {
    data: {
        pincode: router.getParams().pinCode,
    },
    onInit() {
        console.info('getParams: ' + router.getParams());
    },
    onConfirm() {
        console.info('click inpout confirm numbs'+ numbs);
        console.info('click inpout confirm');
        numbs = numbs + 1;
        console.info('click inpout confirm numbs'+ numbs);
        console.info('click confirm start');
        var input=  this.$element('input').value;
        console.info('click confirm end');
        console.info('input: ' + input);
        console.info('pincode: ' + pincode);
        callNativeHandler(input, "0");
    },
    onCancel() {
        console.info('click cancel');
        callNativeHandler("EVENT_CONFIRM", "1");
    }
}