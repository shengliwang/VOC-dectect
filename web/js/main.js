function changeScalar(currentNode){
  currentNode.setAttribute("class", "timetype_selected");
  currentNode.setAttribute("myTestAttribute", "value hello world");
  let n = currentNode.parentNode.firstChild;
  for (; n; n = n.nextSibling){
    if (n.nodeType === 1 && n != currentNode){
      n.setAttribute("class", "timetype_notselected");
    }
  }
}



function mytree(elem){
  this.elem = elem;
}
