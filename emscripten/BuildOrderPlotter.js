let boxHeight = 25;
let boxHeightBuffer = 4;
let xScale = 0.15;
let yScale = 1;

function GetTime(frames) {
    let minutes = "" + parseInt(frames / (24 * 60)) + "m";
    let seconds = "" + parseInt((frames / 24) % 60) + "s";
    return minutes + " " + seconds;
}

function GetCostString(type) {
    let m = AllTypes[type].mineralCost;
    let g = AllTypes[type].gasCost;
    let cost = m + "m";
    if (g > 0) { cost += " " + g + "g"}
    return cost;
}

function pad(num, size) {
    let str = num + "";
    while (str.length < size) str = "0" + str;
    return str;
}


function SetFromConfigString(str) {

    let arr = str.split(',')
    
    let i = 0;
    for (let pid=1; pid<=3; pid++) {
        DeleteAll("S" + pid); 
        let state = document.getElementById("S" + pid);

        while (true) {
            if (arr[i] == "X") { i++; break; }
            let newOption = document.createElement("option");
            newOption.text = arr[i];
            state.add(newOption);
            i++;
        }
    }

    for (let pid=1; pid<=3; pid++) {
        DeleteAll("BO" + pid); 
        let state = document.getElementById("BO" + pid);

        while (true) {
            if (arr[i] == "X") { i++; break; }
            let newOption = document.createElement("option");
            newOption.text = arr[i];
            state.add(newOption);
            i++;
        }
    }
}

function getConfigString() {
    let config = "";

    for (let pid=1; pid<=3; pid++) {

        let state = document.getElementById("S" + pid);
        for (let i=0; i<state.options.length; i++) {
            config += state.options[i].text + ",";
        }

        config += "X,";
    }

    for (let pid=1; pid<=3; pid++) {

        let state = document.getElementById("BO" + pid);
        for (let i=0; i<state.options.length; i++) {
            config += state.options[i].text + ",";
        }

        config += "X";
        if (pid < 3) { config += ",";}
    }

    return config;
}


function CopyURL() {
    let text = 'http://www.cs.mun.ca/~dchurchill/boss/?config=' + this.getConfigString();
    let input = document.createElement('textarea');
    input.innerHTML = text;
    document.body.appendChild(input);
    input.select();
    let result = document.execCommand('copy');
    document.body.removeChild(input);
    alert('URL Copied To Clipboard\n\n' + text)
    return result;
}

function GetBuildOrderItemDiv(buildOrder, index) {
    
    let data = plotData[buildOrder].buildOrder[index];
    
    let x = data[1] * xScale;
    let y = data[5] * (boxHeight + boxHeightBuffer) * yScale;
    let w = (data[2] - data[1]) * xScale;
    let h = boxHeight * yScale;
    let text = data[0];
    let color = data[6];

    let title = data[0] + "\n";
    title += "    Cost: " + GetCostString(data[0]) + "\n";
    title += "    Build: " + AllTypes[data[0]].buildTime + " (" + GetTime(AllTypes[data[0]].buildTime) + ")\n\n";

    title += "State\n";
    title += "    Start: " + data[1] + " (" + GetTime(data[1]) + ")\n";
    title += "    End: " + data[2] + " (" + GetTime(data[2]) + ")\n"
    title += "    Minerals: " + parseInt(data[3]) + "\n"
    title += "    Gas: " + parseInt(data[4]) + "\n"

    let p = '<p style=\'position:absolute;\'>' + text + '</p>';
    
    let div = '<div title="' + title + '" class=\"buildOrderItem\" style=\'';
    div += 'top:' + y + ';';
    div += 'left:' + x + ';'
    div += 'width:' + w + 'px;';
    div += 'height:' + h + 'px;';
    //div += 'border:1px solid #000;';
    div += 'background-color:' + color + ';';
    div += '\'>';
    div += "<div class='centered'>" + text + "</div>";
    div += '</div>';
    return div;
}

function GetVerticalLine(x, y, w, h, color) {
    let div = '<div style=\'';
    div += 'top:' + y + ';';
    div += 'left:' + x + ';'
    div += 'width:' + w + 'px;';
    div += 'height:' + h + 'px;';
    //div += 'background-color:' + color + ';';
    div += 'position:absolute;';
    div += 'border:1px dashed #dddddd;';
    div += '\'>';
    div += '</div>';
    return div;
}

function DrawBuildOrderPlots() {
    
    let maxLayer = 0;
    let currentY = 0;
    for (let p = 0; p < plotData.length; ++p)
    {
        let bo = plotData[p].buildOrder;
    
        // figure out the dimensions of this plot
        let maxX = 0;
        let maxY = 0;
        for (let i=0; i < bo.length; ++i) {
            maxX = Math.max(maxX, bo[i][2]) * xScale;
            maxY = Math.max(maxY, (bo[i][5] + 1) * (boxHeight + boxHeightBuffer) * yScale);
        }
    
        let boDiv = '<div style=\'';
        boDiv += 'width:' + maxX + ';';
        boDiv += 'height:' + maxY + ';';
        boDiv += 'position:absolute;';
        boDiv += 'border:1px solid #ccc;';
        boDiv += 'border-style:dashed;';
        boDiv += 'top:' + currentY + '\'>';
        currentY += (maxY + 50);

        for (let i=1440*xScale; i < maxX; i+=1440*xScale) {
            boDiv += GetVerticalLine(i, -20, 0, maxY + 20, '#000000');
        }
        for (let i=0; i < bo.length; ++i) {
            boDiv += GetBuildOrderItemDiv(p, i);
        }
        
        boDiv += '</div>';
        $('#drawArea').append(boDiv);
    }
}