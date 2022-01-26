import router from '@ohos.router'

var times = 60;
export default {
    data: {
        message: router.getParams().targetPkgName,
    },
    onInit() {
        console.info('getParams: ' + router.getParams());
        this.timeRemaining = times;
		const next = () => {
			--this.timeRemaining;
			if (this.timeRemaining > 0) {
				setTimeout(next, 1000)
			} else {
				onCancel();
			}
		}
		next()
    },
    onConfirm() {
        console.info('click confirm');
        callNativeHandler("EVENT_CONFIRM", "0");
    },
    onCancel() {
        console.info('click cancel');
        callNativeHandler("EVENT_CANCEL", "1");
    }
}