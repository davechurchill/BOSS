function GetBOSSJSON() {
    
    let str = "{ \"BuildOrders\" : [" 
    let builds = 3;

    let obj = {
        BuildOrders : []
    };

    for (let b=1; b<=builds; b++) {
        let startList = document.getElementById('S'  + b);
        let boList    = document.getElementById('BO' + b);
        // if this build order or starting state is empty, skip it
        if (startList.options.length == 0) { continue; }
        if (boList.options.length == 0) { continue; }

        let boObj = {
            Name : "Build Order " + b,
            State : {
                minerals : parseInt(document.getElementById('M' + b).value),
                gas : parseInt(document.getElementById('G' + b).value),
                units : []
            },
            BuildOrder : []
        }

        // calculate the unit count types from the starting units
        for (let s=0; s < startList.options.length; s++) {
            let type = startList.options[s].text;
            let found = false;
            for (let t=0; t < boObj.State.units.length; t++) {
                if (boObj.State.units[t][0] == type) {
                    boObj.State.units[t][1] += 1;
                    found = true;
                }
            }
            if (!found) { boObj.State.units.push([type, 1]); }
        }

        for (let s=0; s < boList.options.length; s++) {
            let type = boList.options[s].text;
            boObj.BuildOrder.push(type);
        }

        obj.BuildOrders.push(boObj);
    }

    return JSON.stringify(obj);
}

function SetInitialBuildOrder(listID, bo) {
    let listb = document.getElementById(listID);
    for (let i=0; i<bo.length; i++) {
        let newOption = document.createElement("option");
        newOption.text = bo[i];
        listb.options.add(newOption, null);
    }
}

function SetUnitTypes(race) {
    DeleteAll('UnitTypes');
    let listb = document.getElementById("UnitTypes");
    for (prop in BWData["Types"]) {
        let type = BWData["Types"][prop];
        if (type["race"] == race && type["name"] != "Larva") {
            let newOption = document.createElement("option");
            newOption.text = type["name"];
            listb.options.add(newOption, null);
        }
    }
    SetColors(['UnitTypes', 'BO1', 'BO2', 'BO3', 'S1', 'S2', 'S3']);
}

function SetColors(listIDs) {
    for (let l=0; l<listIDs.length; l++)
    {
        let listb = document.getElementById(listIDs[l]); 
        for (let i=0; i<listb.options.length; i++)
        {
            let type = listb.options[i].text;
            if (AllTypes[type]["isWorker"]) {
                listb.options[i].style.background = '#d9ffff';
            } else if (AllTypes[type]["isSupplyProvider"]) {
                listb.options[i].style.background = '#fff9d9';
            } else if (AllTypes[type]["isRefinery"]) {
                listb.options[i].style.background = '#d9ffd9';
            } else if (AllTypes[type]["isBuilding"]) {
                listb.options[i].style.background = '#f1dfdf';
            } else {
                listb.options[i].style.background = '#bebebe';
            }
        }
    }
}

function SelectAll(listID) {  
    let listb = document.getElementById(listID); 
    for (let i=0; i<listb.options.length; i++) {
        listb.options[i].selected = true;
    }
    SetColors(['UnitTypes', 'BO1', 'BO2', 'BO3', 'S1', 'S2', 'S3']);
}  

function DeleteAll(listID) {  
    let listb = document.getElementById(listID); 
    let size = listb.size; 
    while (listb.options.length > 0)
    {
        listb.options.remove(0);
    }
    SetColors(['UnitTypes', 'BO1', 'BO2', 'BO3', 'S1', 'S2', 'S3']);
}  

function DeleteSelected(listID) {  
    let listb = document.getElementById(listID);  
    let len = listb.options.length;  
    for (let i = listb.options.length-1 ; i >= 0 ; i--) {  
        if (listb.options[i].selected == true) {  
            listb.options.remove(i);  
        }  
    }
    SetColors(['UnitTypes', 'BO1', 'BO2', 'BO3', 'S1', 'S2', 'S3']);
}  

function CopySelected(sourceID, destIDs) {
    for (let d=0; d<destIDs.length; d++) {
        let src = document.getElementById(sourceID);
        let dest = document.getElementById(destIDs[d]);
        for(let i=0; i < src.options.length; i++) {
            if(src.options[i].selected == true) {
                let option = src.options[i];
                // check to see if the destination has other race units
                let race = AllTypes[option.text].race;
                for (let j=0; j<dest.options.length; j++) {
                    if (AllTypes[dest.options[j].text].race != race) {
                        alert('Destination cannot contain unit(s) of another race\n\nTrying to place ' + race + " unit(s) with " + AllTypes[dest.options[j].text].race + " unit(s)");
                        good = false;
                        return;
                    }
                }
                let newOption = document.createElement("option");
                newOption.value = option.value;
                newOption.text = option.text;
                //newOption.selected = true;
                try { dest.add(newOption, null); }
                catch(error) { dest.add(newOption); }
            }
        }
    }
    SetColors(['UnitTypes', 'BO1', 'BO2', 'BO3', 'S1', 'S2', 'S3']);
}

function MoveSelected(listID, increment) {

    let listbox = document.getElementById(listID);
    let selIndex = listbox.selectedIndex;

    if(selIndex == -1) { return; }

    if((selIndex + increment) < 0 ||
        (selIndex + increment) > (listbox.options.length-1)) {
        return;
    }

    let selValue = listbox.options[selIndex].value;
    let selText = listbox.options[selIndex].text;
    listbox.options[selIndex].value = listbox.options[selIndex + increment].value
    listbox.options[selIndex].text = listbox.options[selIndex + increment].text
    listbox.options[selIndex + increment].value = selValue;
    listbox.options[selIndex + increment].text = selText;
    listbox.selectedIndex = selIndex + increment;
    SetColors(['UnitTypes', 'BO1', 'BO2', 'BO3', 'S1', 'S2', 'S3']);
}