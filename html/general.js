function adjustContentHeight() {
    var element = document.getElementById('content');
    var deviceHeight = window.innerHeight;
    var elementHeight = element.offsetHeight;
    if (elementHeight < deviceHeight) {
        element.style.height = deviceHeight - 90 + 'px';
    } else {
        element.style.height = ''; // Remove the height attribute
    }
}

// Function to handle mutations in the content div or its subtree
function handleMutation(mutationsList, observer) {
    for(var mutation of mutationsList) {
        if (mutation.type === 'attributes' || mutation.type === 'childList') {
            adjustContentHeight();
            break;
        }
    }
}

// Create a MutationObserver instance
var observer = new MutationObserver(handleMutation);

// Start observing the content div and its subtree
var content = document.getElementById('content');
observer.observe(content, { attributes: true, childList: true, subtree: true });

// Initial call to adjustContentHeight
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