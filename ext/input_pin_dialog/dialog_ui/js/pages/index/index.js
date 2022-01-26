import router from '@ohos.router'

var numbs = 0;
export default {
    data: {
        pincode: router.getParams().pinCode,
    },
    onInit() {
        console.info('getParams: ' + router.getParams());
    },
    onConfirm() {
        console.info('click confirm');
        numbs = numbs + 1;
        if (numbs < 3){
            var input=  document.getElementById("input").value;
            console.info('input: ' + input);
            console.info('pincode: ' + pincode);
            if (input == pincode){
                console.info('pincode: = input');
                callNativeHandler(input, "0");
            } else {
                console.info('pincode: != input');
                //刷新页面，提示输入错误
            }
        } else {
            console.info('click cancel');
            callNativeHandler("EVENT_CONFIRM", "1");
        }
    },
    onCancel() {
        console.info('click cancel');
        callNativeHandler("EVENT_CONFIRM", "1");
    }
}