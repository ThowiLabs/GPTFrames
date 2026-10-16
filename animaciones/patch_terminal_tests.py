from pathlib import Path
p=Path("tests/mcp_test.cpp")
x=p.read_text()
for before,after in [
 ('>=61,"Assets scan missing"','>=65,"Assets scan missing"'),
 ('==61,"Expected 61 unique animations"','==65,"Expected 65 unique animations"'),
 ('facets["total"]==61','facets["total"]==65')
]:
 assert x.count(before)==1,repr(before)
 x=x.replace(before,after)
marker='        const auto types=service.run_tool("list_animations",{{"type","servidor"}});'
assert x.count(marker)==1
code="""        const auto terminals=service.run_tool("list_animations",{{"category","Terminales Linux"}});
        require(terminals["total"]==4,"Expected four detailed Linux terminals");
        const auto bash=service.run_tool("describe_animation",{{"id","terminal-bash-nginx"}});
        require(bash["editable"].contains("command") && bash["motion"].is_array(),
                "Terminal must support an editable command");
        const auto boot=service.run_tool("search_animations",{{"query","systemd arranque"}});
        require(boot["total"]>=1,"Boot assets should be searchable");
"""
p.write_text(x.replace(marker,code+marker))
print("MCP tests updated: 65 resources including four terminals")
