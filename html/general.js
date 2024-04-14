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
