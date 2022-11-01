
/*
quick start
https://docs.webix.com/datatree__quick_start.html
api refercences
https://docs.webix.com/api__link__ui.tree_ondblclick_config.html
*/

// 初始化画布
let myChart = null;
let chartStyle='line';

// 初始化导航树
const baseUrl = "./data";
let tree = null;

/*更新图表的函数*/
function updateChart(data){
  const chartdata ={
      labels: data.dataX,
      datasets:[
          {
              label: data.descriptions[0],
              backgroundColor: 'rgb(255, 99, 132)',
              borderColor: 'rgb(255, 99, 132)',
              data: data.dataY[0],
              tension: 0.2,
          },
          {
              label: data.descriptions[1],
              backgroundColor: "#36a2eb",
              borderColor: "#36a2eb",
              data: data.dataY[1],
              tension: 0.2,
          },
      ],
  };
  config = {
      type: chartStyle,
      data: chartdata,
      responsive: true,
      options: {
        plugins: {
          title: {
            display: true,
            text: data.label,
          },
        },
        scales: {
          x: {
            display: true,
            title: {
              display: true,
              text: "单位: " + data.xUnit,
            }
          },
          y: {
            display: true,
            title: {
              display: true,
              text: data.yUnit,
            },
            suggestedMin: 0,
            suggestedMax: 7
          }
        },
      }
  };

  if (myChart != null && myChart instanceof Chart)
  {
      myChart.destroy();
      myChart = null;
  }

  myChart = new Chart(document.getElementById("chart"), config);
}

/*
* 创建树的递归函数 
*/
function treeAddElementRecsive(base_url, treeParentId)
{
  const url = base_url+ "/config.json";
  const httpRequest = new XMLHttpRequest();
  httpRequest.onreadystatechange = function() {
    if (this.readyState == XMLHttpRequest.DONE
      && this.status == 200)
    {
      treeAddNode(treeParentId, base_url, JSON.parse(this.responseText));
    }
  };
  httpRequest.open("GET", url, true);
  httpRequest.send();
}

let id_counter = 0;
function treeAddNode(treeParentId, base_url, data){
  id_counter ++;
  tree.add(
      {id:"idhash_" + String(id_counter), value: "显示" + data.dirUint + "图表", 
      open: false, $css: "filestyle"},
       0, treeParentId
  );

  for (i in data.childDirs){
    tree.add(
      {id: data.childDirs[i], value: data.childDirs[i]+data.childDirsUnit, 
        open:true, $css: "dirstyle"},
      -1, treeParentId
    );

    const newurl = base_url + "/" + data.childDirs[i];
    treeAddElementRecsive(newurl, data.childDirs[i]);
  }
}

/* 初始化树的函数 */
function initTree(base_url){
    tree = webix.ui({
      container:"tree",
      view:"tree",
      type:"lineTree",
      data: [
          {id: "root", value: "root", open: true}
      ]
    });

    const url = base_url+ "/config.json"
    const httpRequest = new XMLHttpRequest();
    httpRequest.onreadystatechange = function() {
      if (this.readyState == XMLHttpRequest.DONE
        && this.status == 200)
      {
        treeAddNode("root", base_url, JSON.parse(this.responseText));
        /*初始化默认的图表（第一个）*/
        updateChart(JSON.parse(this.responseText));
      } 
    };
    httpRequest.open("GET", url, true);
    httpRequest.send();
}


/* 调用函数初始化树 */
initTree(baseUrl);

/* onItemDblClick */
/*
 * 注册文件树的回调函数（单击，双击的话，要注册上面的事件）
*/
tree.attachEvent("onItemClick", function(id) {
  if (id.match("^idhash_.*") == null)
  {
    return ;
  }

  let parentId = null;
  parentId = tree.getParentId(id);
 
  let url = "/config.json";
  while(parentId != "root")
  {
    url = "/" + parentId + url;
    parentId = tree.getParentId(parentId)
  }

  url = baseUrl + url;

  const httpRequest = new XMLHttpRequest();
  httpRequest.onreadystatechange = function() {
    if (this.readyState == XMLHttpRequest.DONE
        && this.status == 200){
      const data = JSON.parse(this.responseText);
      /* 更新图表 */
      updateChart(data)
    }
  };
  httpRequest.open("GET", url, true);
  httpRequest.send();  
});


/*复选框功能*/
function inputBoxContentChangeCallBack()
{
  let elements = document.querySelectorAll('[name="charttype"]');

  for(i =0; i < elements.length; ++i)
  {
    if (elements[i].checked){
      chartStyle = elements[i].value;
    }
  }
  myChart.type = chartStyle;
  // myChart.update();
  // myChart.render();
  // console.log(myChart)
}