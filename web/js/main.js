
/*
quick start
https://docs.webix.com/datatree__quick_start.html
api refercences
https://docs.webix.com/api__link__ui.tree_ondblclick_config.html
*/

// 初始化画布
let myChart = null;

const baseUrl = "./data"


// 初始化导航树
let tree = null;
// webix.ui({
//   container:"tree",
//   view:"tree",
//   select:true,
//   data: [
//     {id:"root", value:"Cars", open:true, data:[
//       { id:"1", open:true, value:"Toyota", data:[
//         { id:"1.1", value:"Avalon" },
//         { id:"1.2", value:"Corolla" },
//         { id:"1.3", value:"Camry" }
//       ]},
//       { id:"2", value:"Skoda", open:true, data:[
//         { id:"2.1", value:"Octavia" },
//         { id:"2.2", value:"Superb" }
//       ]},
//       { id:"2", value:"Skoda"
//       }
//     ]}
//   ]
// });

tree = webix.ui({
  container:"tree",
  view:"tree",
  type:"lineTree",
  data: [
      {id: "root", value: "root", open: true}
  ]
});

function updateChart(data){
  const chartdata ={
      labels: data.dataX,
      datasets:[
          {
              label: data.descriptions[0],
              backgroundColor: 'rgb(255, 99, 132)',
              borderColor: 'rgb(255, 99, 132)',
              data: data.dataY[0],
          },
          {
              label: data.descriptions[1],
              backgroundColor: "#36a2eb",
              borderColor: "#36a2eb",
              data: data.dataY[1],
          },
      ],
  };
  config = {
      type: "line",
      data: chartdata,
      options: {}
  };

  if (myChart != null && myChart instanceof Chart)
  {
      myChart.destroy();
      myChart = null;
  }

  myChart = new Chart(document.getElementById("chart"), config);
}


tree.attachEvent("onItemDblClick", function(id) {
  url = tree.getItemNode(id).getAttribute("myUrl");

  console.log("myUrl is: " + url);

  const httpRequest = new XMLHttpRequest();
  httpRequest.onreadystatechange = function() {
    if (this.readyState == XMLHttpRequest.DONE
        && this.status == 200){
      const data = JSON.parse(this.responseText);
      updateChart(data)
    }
  };
  httpRequest.open("GET", url, true);
  httpRequest.send();  
});

initTree(baseUrl);

function treeAddElementRecsive(baseUrl, treeParentId)
{
  const url = baseUrl+ "/config.json";
  const httpRequest = new XMLHttpRequest();
  httpRequest.onreadystatechange = function() {
    httpRequestCallback(this, treeParentId);
  };
  httpRequest.open("GET", url, true);
  httpRequest.send();
}

function httpRequestCallback(handle, treeParentId){
  if (handle.readyState == XMLHttpRequest.DONE){
      if (handle.status == 200){
          data = JSON.parse(handle.responseText);
          const id = tree.add(
            {id:"", value: "显示" + data.dirUint + "图表", open: false,
              $css: "filestyle"},
            0, treeParentId
          );

          const ele = tree.getItemNode(id);
          ele.setAttribute("myUrl", handle.responseURL);

          for (i in data.childDirs){
            console.log("i=" + i);
            tree.add(
              {id: data.childDirs[i], value: data.childDirs[i]+data.childDirsUnit, 
                open:true, $css: "dirstyle"},
              -1, treeParentId
            );
            const base = (new URL(".", handle.responseURL)).toString();
            const newurl = base + data.childDirs[i];
            treeAddElementRecsive(newurl, data.childDirs[i]);
          }
      } else {
          console.log("error to get url: " + (handle.responseURL) );
      }
  }
}


function initTree(base_url){
    const url = base_url+ "/config.json"
    const httpRequest = new XMLHttpRequest();
    httpRequest.onreadystatechange = function() {
      httpRequestCallback(this, "root");
    };
    httpRequest.open("GET", url, true);
    httpRequest.send();
}



