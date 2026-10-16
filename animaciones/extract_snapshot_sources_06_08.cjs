#!/usr/bin/env node
// Extract data-only snapshots from three user-submitted animation labs.
// Runs only the module data arrays in a restricted vm context, not their app boot code.
const fs=require('fs');
const vm=require('vm');
const acorn=require('acorn');
const path=require('path');
const root=path.resolve(__dirname);
const inputs=[
 ["architecture","source_architecture_06.html","ARCHITECTURES"],
 ["runtime","source_runtime_07.html","MODULES"],
 ["languages","source_languages_08.html","MODULES"],
];
const data=[];
for(const [batch,filename,arrayName] of inputs){
 const html=fs.readFileSync(path.join(root,filename),'utf8');
 const source=html.slice(html.indexOf('<script>')+8,html.indexOf('</script>'));
 if(!source)throw Error("JavaScript module not found in "+filename);
 const ast=acorn.parse(source,{ecmaVersion:'latest',sourceType:'script'});
 const declaration=ast.body.find(node=>node.type==='VariableDeclaration'&&node.declarations.some(d=>d.id.name===arrayName));
 if(!declaration)throw Error("Missing module declaration in "+filename);
 const code=source.slice(declaration.start,declaration.end);
 const context=vm.createContext(Object.create(null),{codeGeneration:{strings:false,wasm:false}});
 vm.runInContext("'use strict';"+code+"globalThis.ITEMS="+arrayName+";",context,{timeout:1000,displayErrors:true});
 const len=vm.runInContext("ITEMS.length",context);
 if(len!==10)throw Error("Expected exactly ten "+batch+" items, got "+len);
 for(let i=0;i<len;i++){
  context.index=i;
  const basic=vm.runInContext("(()=>{const m=ITEMS[index];return {id:m.id,num:m.num,name:m.name,subtitle:m.subtitle||m.sub||'',description:m.desc||m.description||m.sub||'',file:m.file||'',category:m.category||''}})()",context,{timeout:150});
  let conf=vm.runInContext("(()=>{const m=ITEMS[index];return m.defaultConfig||m.state||{}})()",context,{timeout:150});
  conf=JSON.parse(JSON.stringify(conf));
  let codeLines=[];
  if(batch==="architecture") {
    codeLines=vm.runInContext("ITEMS[index].code",context,{timeout:150}).split(/\r?\n/);
  }else if(batch==="runtime") {
    context.cfg=conf;
    codeLines=vm.runInContext("ITEMS[index].codeTemplate(cfg)",context,{timeout:150});
  }else{
    context.cfg=conf;
    codeLines=vm.runInContext("ITEMS[index].getCode(cfg).map(x=>x.html)",context,{timeout:150});
  }
  const steps=batch==="runtime"?vm.runInContext("ITEMS[index].stepsCount",context):5;
  if(steps<3||steps>12)throw Error("Invalid steps "+basic.name);
  const snapshots=[];
  for(let k=0;k<steps;k++){
    context.step=k;context.progress=(k+0.03)/(steps-0.01);
    let snap;
    if(batch==="architecture") snap=vm.runInContext(
     "(()=>{let svg={innerHTML:''};ITEMS[index].render(svg,progress);return {svg:svg.innerHTML,log:ITEMS[index].phases?.filter(p=>p.at<=progress).map(p=>p.label).slice(-1)[0]||'Procesando arquitectura',active:step}})()",context,{timeout:150});
    else if(batch==="runtime")snap=vm.runInContext(
     "(()=>{let svg={innerHTML:''},logs=[];const state=ITEMS[index].renderStep(step,cfg,svg,(x)=>{logs.push(...x)});return{svg:svg.innerHTML,log:logs.slice(-2).join('  •  '),active:state.activeLine}})()",context,{timeout:150});
    else snap=vm.runInContext(
     "(()=>{return{svg:ITEMS[index].renderSVG(cfg,progress),log:ITEMS[index].getLogs(cfg,progress).slice(-1)[0],active:step}})()",context,{timeout:150});
    if(typeof snap.svg!=="string"||snap.svg.length<30)throw Error("No visual in "+basic.name+" phase "+k);
    snapshots.push(snap);
  }
  data.push({...basic,batch,configuration:conf,codeLines,snapshots});
 }
}
const out=path.join(root,"snapshot_sources_06_08.json");
fs.writeFileSync(out,JSON.stringify(data));
console.log("Extracted",data.length,"presets,",data.reduce((a,b)=>a+b.snapshots.length,0),"distinct visual states",fs.statSync(out).size,"bytes");
for(const v of data)console.log(v.batch,v.id,v.name,v.snapshots.length,Object.keys(v.configuration).join(","));
