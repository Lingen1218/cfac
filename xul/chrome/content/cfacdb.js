Components.utils.import("resource://gre/modules/AddonManager.jsm");

var cfacdb = {
    ini_id: -1,
    fin_id: -1,
    
    logger: function(level, msg)
    {
        var prefix;

        switch (level) {
        case "err":
            prefix = "E";
            break;
        case "wrn":
            prefix = "W";
            break;
        case "inf":
            prefix = "I";
            break;
        case "dbg":
            if (!this.logDebug) {
                return;
            }
            prefix = "D";
            break;
        default:
            prefix = "?";
            break;
        }

        this.alert(prefix + ": " + msg);
    },
    
    setClassParams: function(class_name, value)
    {
        var es = document.getElementsByClassName(class_name);
        for (var i = 0; i < es.length; i++) {
            es[i].textContent = value;
        }
    },
    
    init: function(db_path)
    {
        var dsources = "file://" + db_path;

        var e;
        e = document.getElementById("levels-ini");
        e.datasources = dsources;
        
        e = document.getElementById("levels-fin");
        e.datasources = dsources;
        
        e = document.getElementById("rtransitions");
        e.datasources = dsources;
        
        e = document.getElementById("species");
        e.datasources = dsources;
        e.view.selection.select(0);
    },

    openCB: function()
    {
        var Cc = Components.classes;
        var Ci = Components.interfaces;

        const nsIFilePicker = Ci.nsIFilePicker;

        var fp;
        if (!this.fpicker) {
            fp = Cc["@mozilla.org/filepicker;1"].createInstance(nsIFilePicker);
            fp.init(window, "Open File", nsIFilePicker.modeOpen);
            fp.appendFilter("cFAC SQLite databases", "*.sqlite; *.db");
            fp.appendFilters(nsIFilePicker.filterAll);
            this.fpicker = fp;
        } else {
            fp = this.fpicker;
        }

        fp.displayDirectory = this.lastDir;

        var rv = fp.show();

        if (rv == nsIFilePicker.returnOK) {
            this.setBusyCursor(true);
            
            var res;
            
            var file = fp.file;
            this.lastDir = file.parent;
            
            this.init(file.path);
            
            this.setBusyCursor(false);
            
            return res;
        } else {
            return false;
        }
        
    },
    
    sessionSelectCB: function(e)
    {
        var tree = e.target;
       
        var selection = tree.view.selection;
        var cellText = tree.view.getCellText(tree.currentIndex,  
                            tree.columns.getColumnAt(0));
        var sid;
        if (cellText) {
            sid = parseInt(cellText);
        } else {
            sid = 0;
        }
        
        this.setClassParams("sid", sid);
        
        document.getElementById("levels-ini").builder.rebuild();
        document.getElementById("levels-fin").builder.rebuild();
    },
    
    levelSelectCB: function(e)
    {
        var tree = e.target;
       
        var selection = tree.view.selection;
        var cellText = tree.view.getCellText(tree.currentIndex,  
                            tree.columns.getColumnAt(0));
        var id;
        if (cellText) {
            id = parseInt(cellText);
        } else {
            id = -1;
        }
        
        var class_name;
        
        if (tree.id == "levels-ini") {
            this.ini_id = id;
            class_name = "ini_id";
        } else {
            this.fin_id = id;
            class_name = "fin_id";
        }
        
        console.log(this.ini_id + " -> " + this.fin_id);
        
        this.setClassParams(class_name, id);
        
        document.getElementById("rtransitions").builder.rebuild();
    },
    
    neleSelectCB: function(e)
    {
        var el = e.target;
        var nele = parseInt(el.value);
        
        this.setClassParams("nele", nele);
        
        document.getElementById("levels-ini").builder.rebuild();
        document.getElementById("levels-fin").builder.rebuild();
    },
    
    deltaNeleCB: function(e)
    {
        var el = e.target;
        var dnele = parseInt(el.value);
        this.setClassParams("dnele", dnele);
        
        document.getElementById("levels-fin").builder.rebuild();
    },
    
    helpCB: function()
    {
        var url = "http://plasma-gate.weizmann.ac.il/cfacdb/xul/";
        var gBrowser = window.opener.gBrowser;

        if (gBrowser) {
            gBrowser.selectedTab = gBrowser.addTab(url);  
        }
    },
    
    aboutCB: function()
    {
        var httpBASE = this.httpBASE;
        AddonManager.getAddonByID("cfacdb@plasma-gate.weizmann.ac.il",
            function(aAddon) {
                var str = aAddon.name + " version: " + aAddon.version;
                str += "\nCreated by: " + aAddon.creator.name;
                // str += "\nREST Base: " + httpBASE;
                cfacdb.alert(str);
            });
    },
    setBusyCursor: function(onoff)
    {
        window.setCursor(onoff ? "wait" : "auto");
    }
};
