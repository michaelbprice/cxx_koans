function evaluateInput (inputElement)
{
    // TODO: Execute AJAX instead...
    var theDiv = inputElement.parentNode;
    
    var currentClass = theDiv.className;
    
    switch (currentClass)
    {
     case "passing": theDiv.className = "failing"; break;
     case "failing": theDiv.className = "passing"; break;
     default: break;
    }
    
    inputElement.value = inputElement.name;
}

function solveInput (inputElement, e)
{
    if (e.altKey && e.keyCode == 27 /*ESC*/)
    {
        // TODO: Execute AJAX instead...
        inputElement.value = "SOLVED"
    }
}
