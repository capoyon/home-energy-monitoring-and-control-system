function adjustContentHeight(elementHeight) {
    if(elementHeight>0){
        element.style.height = elementHeight + 'px';
    } else {
        var element = document.getElementById('content');
        var deviceHeight = window.innerHeight;
        var elementHeight = element.offsetHeight;
        if (elementHeight < deviceHeight) {
            element.style.height = deviceHeight - 90 + 'px';
        } else {
            element.style.height = elementHeight + 'px';
        }
    }
}
window.addEventListener('resize', adjustContentHeight);
adjustContentHeight();


// update time
function updateTime(epochTime, use24HourFormat = false) {
    const date = new Date(epochTime * 1000);
    let options = { weekday: 'short', month: 'short', day: 'numeric', hour: '2-digit', minute: '2-digit' };

    if (use24HourFormat) {
        options.hour12 = false;
    }

    const timeString = date.toLocaleString('en-US', options).replace(/,/g, '');
    document.getElementById('time').innerText = timeString;
}

updateTime(Date.now() / 1000);
setInterval(() => updateTime(Date.now() / 1000), 1000);