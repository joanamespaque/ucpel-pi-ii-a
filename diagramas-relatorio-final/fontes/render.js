const { chromium } = require('/opt/node22/lib/node_modules/playwright');
(async()=>{const b=await chromium.launch();const p=await b.newPage({deviceScaleFactor:2.5});
for (const f of process.argv.slice(2)){await p.goto('file://'+process.cwd()+'/'+f+'.html');const el=await p.$('#d');await el.screenshot({path:f+'.png'});}
await b.close();})();
