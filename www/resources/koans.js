function evaluateExercise (exerciseElement)
{
    var inputs = [];

    var inputElements = exerciseElement.getElementsByTagName("input");

    for (var i = 0; i < inputElements.length; i++)
    {
        inputs.push(inputElements[i].value);
    } 

    var jsonData = {"exercise": exerciseElement.id, "inputs": inputs};

    var xmlhttp = new XMLHttpRequest();

    xmlhttp.onreadystatechange = function ()
    {
        if (xmlhttp.status == 200)
        { 
            markExercisePassing(exerciseElement);
        }
        else
        {
            markExerciseFailing(exerciseElement);
        }
    }

    alert(exerciseElement.parentNode);
    alert(exerciseElement.parentNode.id);

    xmlhttp.open("POST", "/evaluate" + exerciseElement.parentNode.id, true);
    xmlhttp.setRequestHeader("Content-type", "application/json");
    xmlhttp.send(JSON.stringify(jsonData));
}

function markExerciseFailing (exerciseDiv)
{
    exerciseDiv.className = "failing";
}

function markExercisePassing (exerciseDiv)
{
    exerciseDiv.className = "passing";
}

function solveInput (inputElement, e)
{
    if (e.altKey && e.keyCode == 45 /*INS*/)
    {
        var parts = inputElement.name.split("_");
        var jsonData = {"exercise": parts[1], "input": parts[2]};

        var xmlhttp = new XMLHttpRequest();

        xmlhttp.onreadystatechange = function ()
        {
            inputElement.value = xmlhttp.responseText;
            evaluateExercise(inputElement.parentNode);
        }

        xmlhttp.open("POST", "/solve" + inputElement.parentNode.parentNode.id, true);
        xmlhttp.setRequestHeader("Content-type", "application/json");
        xmlhttp.send(JSON.stringify(jsonData));
    }
}
