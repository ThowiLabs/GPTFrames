#!/usr/bin/env python3
import copy, math, sys, unittest, xml.etree.ElementTree as ET
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from generate_bar_chart import DEFAULT, generate
class Charts(unittest.TestCase):
 def data(self,values,maximum=200,step=50):
  d=copy.deepcopy(DEFAULT);d['categories']=[{'label':f'C{i+1}','value':v,'color':'#22D3EE'} for i,v in enumerate(values)]
  d.update(axis_max=maximum,tick_step=step)
  return d
 def test_numeric_geometry_and_update(self):
  for values,maximum,step in [([0,50,100,200],200,50),([20,45,15,20],50,10),([.25,1,1.5],2,.5)]:
   doc,m=generate(self.data(values,maximum,step))
   svg=ET.fromstring(doc[doc.index('<svg '):doc.index('</svg>')+6])
   for mode in ['wide','image']:
    for i,v in enumerate(values,1):
     rect=svg.find(f".//*[@id='{mode}-bar-{i}']")
     y=float(rect.get('y'));h=float(rect.get('height'))
     self.assertAlmostEqual(y+h,520,places=3)
     self.assertAlmostEqual(h/295,v/maximum,places=5)
   self.assertNotIn('value_1',m['editable'])
   self.assertEqual(m['editable']['image']['type'],'image')
  _,old=generate(self.data([50],200,50));_,new=generate(self.data([100],200,50))
  self.assertEqual(new['geometry'][0]['height'],old['geometry'][0]['height']*2)
 def test_invalid_data_rejected(self):
  for value in [-1,201,float('nan'),float('inf'),True]:
   with self.subTest(value=value),self.assertRaises(ValueError):generate(self.data([value]))
  for patch in [{'axis_max':0},{'tick_step':0},{'tick_step':30},{'tick_step':1},{'categories':[]},{'summary_mode':'unknown'},{'value_suffix':123}]:
   d=self.data([20]);d.update(patch)
   with self.subTest(patch=patch),self.assertRaises(ValueError):generate(d)
  for color in ['red','#123','url(http://x)']:
   d=self.data([20]);d['categories'][0]['color']=color
   with self.assertRaises(ValueError):generate(d)
 def test_safe_text_and_summary(self):
  d=self.data([20,45,15,20],50,10);d.update(summary_mode='sum',value_suffix=' %',title='<img & "test">')
  doc,m=generate(d)
  self.assertIn('&lt;img &amp; &quot;test&quot;&gt;',doc)
  self.assertIn('20 + 45 + 15 + 20 = 100 %',m['editable']['conclusion']['default'])
  _,m=generate(self.data([0,20]))
  self.assertNotIn('inf',m['editable']['conclusion']['default'])
 def test_cues_fit_and_follow_bars(self):
  for patch in [{'categories':[-1]*4},{'growth':[0]*4},{'summary':3},{'values':[50]*4},{'summary':11.8}]:
   d=self.data([20]*4);d['timing']=patch
   with self.subTest(patch=patch),self.assertRaises(ValueError):generate(d)
  _,m=generate(self.data([20]*4))
  self.assertTrue(all(0<=e['at'] and e['at']+e['duration']<=m['duration'] for e in m['events']))
if __name__=='__main__':unittest.main()
