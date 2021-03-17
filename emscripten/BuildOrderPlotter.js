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

function GetBuildOrderItemDiv(buildOrder, index) {
    
    let data = plotData[buildOrder].buildOrder[index];
    console.log(buildOrder, index, data);
    let x = data[1] * xScale;
    let y = data[5] * (boxHeight + boxHeightBuffer) * yScale;
    let w = (data[2] - data[1]) * xScale;
    let h = boxHeight * yScale;
    let text = data[0];
    let color = data[6];

    let title = data[0] + "\n";
    title += "    Cost: " + GetCostString(data[0]) + "\n";
    title += "    Build: " + GetTime(AllTypes[data[0]].buildTime) + "\n\n";

    title += "State\n";
    title += "    Start: " + GetTime(data[1]) + "\n";
    title += "    End: " + GetTime(data[2]) + "\n"
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
        currentY += (maxY + 30);
        for (let i=0; i < bo.length; ++i) {
            boDiv += GetBuildOrderItemDiv(p, i);
        }
        
        boDiv += '</div>';
        $('#drawArea').append(boDiv);
    }
}