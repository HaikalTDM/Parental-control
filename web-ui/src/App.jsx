import React, { useState, useEffect } from 'react';
import {
  Wifi, WifiOff, Smartphone, Shield, Clock, Home, Activity,
  Zap, Lock, Unlock, PlayCircle, Youtube, Facebook,
  Gamepad2, Instagram, Check, Plus, Trash2, Ban, Globe, ShieldCheck,
  ArrowUp, ArrowDown, Users, ShieldAlert, Loader
} from 'lucide-react';

// --- UI COMPONENTS (Shadcn Style) ---

const Switch = ({ checked, onCheckedChange, colorClass = "bg-slate-900" }) => (
  <button
    type="button"
    role="switch"
    aria-checked={checked}
    onClick={() => onCheckedChange(!checked)}
    className={`
      relative inline-flex h-7 w-12 shrink-0 cursor-pointer items-center rounded-full border-2 border-transparent 
      transition-colors duration-200 ease-in-out focus-visible:outline-none focus-visible:ring-2 
      focus-visible:ring-slate-400 focus-visible:ring-offset-2
      ${checked ? colorClass : 'bg-slate-200'}
    `}
  >
    <span
      className={`
        pointer-events-none block h-6 w-6 rounded-full bg-white shadow-lg ring-0 
        transition-transform duration-200 ease-in-out
        ${checked ? 'translate-x-5' : 'translate-x-0'}
      `}
    />
  </button>
);

const Card = ({ children, className = "" }) => (
  <div className={`bg-white rounded-xl border border-slate-200 shadow-sm ${className}`}>
    {children}
  </div>
);

// --- VIEW COMPONENTS ---

const StatusRing = ({ internetActive, toggleInternet }) => (
  <div className="relative flex justify-center items-center py-6">
    {/* Outer Pulse */}
    <div className={`absolute w-64 h-64 rounded-full opacity-10 transition-colors duration-500 ${internetActive ? 'bg-emerald-400 animate-[pulse_10s_ease-in-out_infinite]' : 'bg-rose-400'}`}></div>
    <div className={`absolute w-52 h-52 rounded-full opacity-20 transition-colors duration-500 ${internetActive ? 'bg-emerald-400' : 'bg-rose-400'}`}></div>

    {/* Main Button */}
    <button
      onClick={toggleInternet}
      className={`relative z-10 w-48 h-48 rounded-full shadow-2xl flex flex-col items-center justify-center transition-all duration-300 transform active:scale-95 border-8
      ${internetActive
          ? 'bg-white border-emerald-50 text-emerald-600 shadow-emerald-200/50'
          : 'bg-white border-rose-50 text-rose-500 shadow-rose-200/50'
        }`}
    >
      <div className={`mb-2 p-3 rounded-full transition-colors duration-300 ${internetActive ? 'bg-emerald-50' : 'bg-rose-50'}`}>
        {internetActive ? <Wifi className="w-8 h-8" /> : <WifiOff className="w-8 h-8" />}
      </div>
      <span className="text-xs font-bold tracking-widest uppercase opacity-50">Master Switch</span>
      <span className="text-2xl font-black transition-all duration-300">{internetActive ? 'ONLINE' : 'OFFLINE'}</span>
    </button>
  </div>
);

const StatCard = ({ icon: Icon, label, value, subValue, colorClass, bgClass }) => (
  <Card className="p-3 flex flex-col justify-between h-24 relative overflow-hidden group">
    <div className={`absolute right-2 top-2 p-1.5 rounded-lg opacity-80 ${bgClass} ${colorClass}`}>
      <Icon className="w-4 h-4" />
    </div>
    <div className="mt-auto">
      <h3 className="text-2xl font-bold text-slate-800 tracking-tight">{value}</h3>
      <p className="text-xs font-medium text-slate-400 uppercase tracking-wider">{label}</p>
      {subValue && <p className="text-[10px] text-slatebangu-400 mt-0.5">{subValue}</p>}
    </div>
  </Card>
);

const Dashboard = ({ internetActive, toggleInternet, devices, customBlockList, allowList, realStats, speed }) => {
  // Calculate Stats
  const onlineDevices = realStats ? realStats.connectedDevices : devices.filter(d => d.status === 'online').length;
  // Count only ACTIVE custom blocks
  const activeCustomBlocks = customBlockList.filter(d => d.active).length;
  const totalBlocked = activeCustomBlocks;
  const totalAllowed = allowList.filter(d => d.active).length;



  return (
    <div className="space-y-6 animate-in fade-in slide-in-from-bottom-4 duration-500">

      {/* Header */}
      <div className="flex justify-between items-center px-1">
        <div>
          <h1 className="text-2xl font-bold text-slate-900">System Overview</h1>
          <div className="flex items-center gap-2 mt-1">
            <span className={`w-2 h-2 rounded-full ${internetActive ? 'bg-emerald-500 animate-pulse' : 'bg-rose-500'}`}></span>
            <p className="text-sm text-slate-500 font-medium">{internetActive ? 'System Operational' : 'Internet Paused'}</p>
          </div>
        </div>
        <div className="w-10 h-10 rounded-full bg-slate-100 flex items-center justify-center border border-slate-200">
          <Activity className="w-5 h-5 text-slate-500" />
        </div>
      </div>

      {/* Main Control */}
      <StatusRing internetActive={internetActive} toggleInternet={toggleInternet} />

      {/* Stats Grid */}
      <div className="grid grid-cols-2 gap-3">
        {/* Devices Stat */}
        <StatCard
          icon={Users}
          label="Online"
          value={onlineDevices}
          subValue={`of ${devices.length} Devices`}
          colorClass="text-blue-600"
          bgClass="bg-blue-50"
        />

        {/* Blocked Stat */}
        <StatCard
          icon={ShieldAlert}
          label="Blocking"
          value={totalBlocked}
          subValue={`${activeCustomBlocks} Domains`}
          colorClass="text-rose-600"
          bgClass="bg-rose-50"
        />

        {/* Allowed Stat */}
        <StatCard
          icon={ShieldCheck}
          label="Allowlist"
          value={totalAllowed}
          subValue="Safe Domains"
          colorClass="text-emerald-600"
          bgClass="bg-emerald-50"
        />

        {/* Speed Stat (Mock) */}
        <StatCard
          icon={Zap}
          label="Speed"
          value={speed}
          subValue="Mbps"
          colorClass="text-amber-600"
          bgClass="bg-amber-50"
        />
      </div>

      {/* Traffic Summary (Visual) */}
      <Card className="p-4 flex items-center justify-between">
        <div className="flex items-center gap-4">
          <div className="p-2 bg-indigo-50 rounded-lg text-indigo-600">
            <Globe className="w-5 h-5" />
          </div>
          <div>
            <p className="text-sm font-bold text-slate-700">Total Data Usage</p>
            <p className="text-xs text-slate-400">Today</p>
          </div>
        </div>
        <div className="text-right">
          <p className="text-lg font-bold text-slate-800">{realStats && realStats.dataUsage ? realStats.dataUsage.total : "0 GB"}</p>
          <div className="flex items-center gap-1 text-emerald-500 text-xs font-bold">
            <ArrowDown className="w-3 h-3" />
            <span>12%</span>
          </div>
        </div>
      </Card>

    </div>
  );
};

const DevicesList = ({ devices, toggleDeviceBlock }) => (
  <div className="space-y-4 animate-in fade-in slide-in-from-bottom-4 duration-500">
    <div className="flex justify-between items-center mb-2 px-1">
      <div>
        <h1 className="text-2xl font-bold text-slate-900">Connected Devices</h1>
        <p className="text-sm text-slate-500 font-medium">Manage access per device</p>
      </div>
      <span className="text-xs font-bold bg-slate-100 px-2.5 py-1 rounded-full text-slate-600">{devices.length} Total</span>
    </div>

    {devices.map((device) => (
      <Card key={device.id} className="p-4 flex items-center justify-between transition-all duration-200">
        <div className="flex items-center gap-4">
          <div className={`w-12 h-12 rounded-xl flex items-center justify-center text-lg transition-colors duration-300
            ${device.blocked ? 'bg-slate-100 text-slate-400' : 'bg-blue-50 text-blue-600'}`}>
            <Smartphone className="w-6 h-6" />
          </div>
          <div>
            <h3 className={`font-bold text-sm transition-colors duration-200 ${device.blocked ? 'text-slate-400' : 'text-slate-900'}`}>
              {device.name}
            </h3>
            <div className="flex items-center gap-2 mt-0.5">
              <span className={`w-2 h-2 rounded-full ${device.status === 'online' ? 'bg-emerald-400' : 'bg-slate-300'}`}></span>
              <p className="text-xs text-slate-400 font-medium">{device.status === 'online' ? 'Online' : 'Offline'}</p>
              <span className="text-slate-300 text-[10px]">•</span>
              <p className="text-xs text-slate-400 font-medium">{device.usage}</p>
            </div>
          </div>
        </div>

        <button
          onClick={() => toggleDeviceBlock(device.id)}
          className={`
            w-10 h-10 rounded-full flex items-center justify-center transition-all duration-200 active:scale-90
            ${device.blocked
              ? 'bg-rose-100 text-rose-600 hover:bg-rose-200'
              : 'bg-slate-100 text-slate-400 hover:bg-slate-200 hover:text-slate-600'}
          `}
        >
          {device.blocked ? <Lock className="w-5 h-5" /> : <Unlock className="w-5 h-5" />}
        </button>
      </Card>
    ))}
  </div>
);

const BlocklistView = ({ customBlockList, addCustomBlock, removeCustomBlock, toggleCustomBlock, applyChanges }) => {
  const [newDomain, setNewDomain] = useState('');
  const [isApplying, setIsApplying] = useState(false);
  const [notification, setNotification] = useState(null);
  const [pendingChanges, setPendingChanges] = useState([]);

  const showNotification = (message, type = 'success') => {
    setNotification({ message, type });
    setTimeout(() => setNotification(null), 3000);
  };

  const handleAdd = () => {
    if (newDomain) {
      // Add to local list immediately
      addCustomBlock(newDomain);
      // Track as pending change
      setPendingChanges(prev => [...prev, { action: 'add', domain: newDomain }]);
      setNewDomain('');
      showNotification(`${newDomain} added to pending changes`, 'success');
    }
  }

  const handleRemove = (id, domain) => {
    // Remove from local list immediately
    removeCustomBlock(id);
    // Track as pending change
    setPendingChanges(prev => [...prev, { action: 'remove', domain }]);
    showNotification(`${domain} removed (pending apply)`, 'success');
  };

  const handleToggle = (id, domain, currentState) => {
    // Toggle in local list immediately
    toggleCustomBlock(id);
    // Track as pending change
    const action = currentState ? 'disable' : 'enable';
    setPendingChanges(prev => [...prev, { action, domain }]);
    showNotification(`${domain} ${action}d (pending apply)`, 'success');
  };

  const handleApply = async () => {
    if (pendingChanges.length === 0) return;

    setIsApplying(true);
    try {
      await applyChanges(pendingChanges);
      setPendingChanges([]);
      showNotification(`Changes applied successfully! DNS reloading...`, 'success');
    } catch (error) {
      showNotification(`Failed to apply changes`, 'error');
    } finally {
      setIsApplying(false);
    }
  };

  return (
    <div className="space-y-6 animate-in fade-in slide-in-from-bottom-4 duration-500">
      <div className="px-1">
        <h1 className="text-2xl font-bold text-slate-900">Edit Blocklist</h1>
        <p className="text-sm text-slate-500 font-medium">Block domains from accessing the internet</p>
      </div>

      {/* Notification */}
      {notification && (
        <div className={`p-4 rounded-xl flex items-center gap-3 animate-in slide-in-from-top-2 ${notification.type === 'success' ? 'bg-emerald-50 text-emerald-700 border border-emerald-200' : 'bg-rose-50 text-rose-700 border border-rose-200'
          }`}>
          {notification.type === 'success' ? (
            <ShieldCheck className="w-5 h-5" />
          ) : (
            <ShieldAlert className="w-5 h-5" />
          )}
          <span className="font-medium text-sm">{notification.message}</span>
        </div>
      )}

      {/* Pending Changes Banner */}
      {pendingChanges.length > 0 && (
        <Card className="p-4 bg-amber-50 border-amber-200">
          <div className="flex items-center justify-between">
            <div className="flex items-center gap-3">
              <Clock className="w-5 h-5 text-amber-600" />
              <div>
                <p className="font-bold text-sm text-amber-900">{pendingChanges.length} Pending Change{pendingChanges.length > 1 ? 's' : ''}</p>
                <p className="text-xs text-amber-700">Click "Save & Apply" to activate changes</p>
              </div>
            </div>
            <button
              onClick={handleApply}
              disabled={isApplying}
              className="px-6 py-3 bg-amber-600 text-white rounded-xl font-bold text-sm hover:bg-amber-700 active:scale-95 transition-all shadow-sm hover:shadow-md flex items-center gap-2 disabled:opacity-50 disabled:cursor-not-allowed"
            >
              {isApplying ? (
                <>
                  <Loader className="w-4 h-4 animate-spin" />
                  Applying...
                </>
              ) : (
                <>
                  <Check className="w-4 h-4" />
                  Save & Apply
                </>
              )}
            </button>
          </div>
        </Card>
      )}

      {/* Add Domain Input */}
      <Card className="p-4">
        <div className="flex gap-2">
          <input
            type="text"
            value={newDomain}
            onChange={(e) => setNewDomain(e.target.value)}
            onKeyPress={(e) => e.key === 'Enter' && handleAdd()}
            placeholder="Enter domain (e.g., example.com)"
            className="flex-1 px-4 py-3 bg-slate-50 border-2 border-slate-200 rounded-xl text-sm font-medium text-slate-700 placeholder:text-slate-400 focus:outline-none focus:border-indigo-400 focus:bg-white transition-all"
          />
          <button
            onClick={handleAdd}
            disabled={!newDomain}
            className="px-6 py-3 bg-indigo-600 text-white rounded-xl font-bold text-sm hover:bg-indigo-700 active:scale-95 transition-all shadow-sm hover:shadow-md flex items-center gap-2 disabled:opacity-50 disabled:cursor-not-allowed"
          >
            <Plus className="w-4 h-4" />
            Add
          </button>
        </div>
      </Card>



      {/* Blocked Domains List */}
      <div className="space-y-2">
        {customBlockList.map((item) => (
          <div key={item.id} className={`bg-white border rounded-xl p-3 flex justify-between items-center shadow-sm transition-all ${item.active ? 'border-rose-200 bg-rose-50/10' : 'border-slate-200 opacity-60'}`}>
            <div className="flex items-center gap-3">
              <div className={`p-2 rounded-lg transition-colors ${item.active ? 'bg-rose-100 text-rose-500' : 'bg-slate-100 text-slate-400'}`}>
                <Globe className="w-4 h-4" />
              </div>
              <span className={`font-medium text-sm transition-colors ${item.active ? 'text-slate-700' : 'text-slate-400 line-through'}`}>
                {item.domain}
              </span>
            </div>

            <div className="flex items-center gap-3">
              <button
                onClick={() => handleRemove(item.id, item.domain)}
                className="text-slate-300 hover:text-rose-500 p-2 transition-colors"
                title="Delete Rule"
              >
                <Trash2 className="w-4 h-4" />
              </button>

              <Switch
                checked={item.active}
                onCheckedChange={() => handleToggle(item.id, item.domain, item.active)}
                colorClass="bg-rose-500"
              />
            </div>
          </div>
        ))}

        {customBlockList.length === 0 && (
          <div className="text-center py-4 text-slate-400 text-xs">No custom blocks yet.</div>
        )}
      </div>
    </div>
  );
};

const AllowlistView = ({ allowList, addAllowDomain, removeAllowDomain, toggleAllowDomain }) => {
  const [newDomain, setNewDomain] = useState('');

  const handleAdd = () => {
    if (newDomain) {
      addAllowDomain(newDomain);
      setNewDomain('');
    }
  }

  return (
    <div className="space-y-6 animate-in fade-in slide-in-from-bottom-4 duration-500">
      <div className="px-1">
        <h1 className="text-2xl font-bold text-slate-900">Edit Allowlist</h1>
        <p className="text-sm text-slate-500 font-medium">Always allow these sites</p>
      </div>

      <div className="bg-blue-50 border border-blue-100 rounded-xl p-4 flex gap-3">
        <div className="bg-blue-100 p-2 rounded-full h-fit text-blue-600">
          <ShieldCheck className="w-5 h-5" />
        </div>
        <p className="text-sm text-blue-800">
          Domains in this list will bypass all blocks and filters. Useful for educational sites.
        </p>
      </div>

      <div className="flex gap-2">
        <input
          type="text"
          value={newDomain}
          onChange={(e) => setNewDomain(e.target.value)}
          placeholder="e.g. schools.edu.my"
          className="flex-1 bg-white border border-slate-200 rounded-xl px-4 py-3 text-sm focus:outline-none focus:ring-2 focus:ring-emerald-500 focus:border-transparent shadow-sm"
        />
        <button
          onClick={handleAdd}
          className="bg-emerald-500 text-white p-3 rounded-xl shadow-lg shadow-emerald-200 active:scale-95 transition-transform"
        >
          <Plus className="w-5 h-5" />
        </button>
      </div>

      <div className="space-y-2">
        {allowList.map((item) => (
          <div key={item.id} className={`bg-white border rounded-xl p-3 flex justify-between items-center shadow-sm transition-all ${item.active ? 'border-emerald-200 bg-emerald-50/10' : 'border-slate-200'}`}>
            <div className="flex items-center gap-3">
              <div className={`p-2 rounded-lg transition-colors ${item.active ? 'bg-emerald-100 text-emerald-600' : 'bg-slate-100 text-slate-400'}`}>
                <Check className="w-4 h-4" />
              </div>
              <span className={`font-medium text-sm transition-colors ${item.active ? 'text-slate-700' : 'text-slate-400 line-through'}`}>
                {item.domain}
              </span>
            </div>

            <div className="flex items-center gap-3">
              <button
                onClick={() => removeAllowDomain(item.id)}
                className="text-slate-300 hover:text-rose-500 p-2 transition-colors"
                title="Delete Rule"
              >
                <Trash2 className="w-4 h-4" />
              </button>

              <Switch
                checked={item.active}
                onCheckedChange={() => toggleAllowDomain(item.id)}
                colorClass="bg-emerald-500"
              />
            </div>
          </div>
        ))}
        {allowList.length === 0 && (
          <div className="text-center py-8 text-slate-400 text-sm">No allowed domains yet.</div>
        )}
      </div>

    </div>
  );
};

// --- MAIN APP ---

const App = () => {
  const [activeTab, setActiveTab] = useState('overview');
  const [internetActive, setInternetActive] = useState(true);

  const [realStats, setRealStats] = useState(null);

  // Data States
  const [devices, setDevices] = useState([]);


  // State now stores objects with { id, domain, active }
  const [customBlockList, setCustomBlockList] = useState([]);

  const [allowList, setAllowList] = useState([]);

  // Handlers
  const toggleInternet = () => setInternetActive(prev => !prev);
  const toggleDeviceBlock = (id) => setDevices(prev => prev.map(d => d.id === id ? { ...d, blocked: !d.blocked } : d));

  // Fetch Stats
  const lastTrafficRef = React.useRef({ rx: 0, tx: 0, time: 0 });
  const [speed, setSpeed] = React.useState("0");

  React.useEffect(() => {
    const fetchStats = async () => {
      try {
        const res = await fetch('/api/stats');
        const data = await res.json();
        setRealStats(data);

        // Calculate Speed
        if (data.traffic) {
          const now = Date.now();
          const currentRx = Number(data.traffic.rx);
          const currentTx = Number(data.traffic.tx);
          const last = lastTrafficRef.current;

          if (last.time > 0) {
            const timeDiff = (now - last.time) / 1000; // Seconds
            if (timeDiff > 0) {
              const bytesDiff = (currentRx + currentTx) - (last.rx + last.tx);
              if (bytesDiff >= 0) {
                const bitsPerSec = (bytesDiff * 8) / timeDiff;
                const mbps = (bitsPerSec / 1000000).toFixed(2);
                setSpeed(mbps);
              }
            }
          }

          lastTrafficRef.current = { rx: currentRx, tx: currentTx, time: now };
        }

        // Update devices list if available
        if (data.devices && Array.isArray(data.devices)) {
          // Map OpenWrt lease object to our UI format
          // OpenWrt lease: { hostname, macaddr, ipaddr, expires }
          const mappedDevices = data.devices.map((d, index) => ({
            id: d.macaddr || index,
            name: d.hostname || d.macaddr || `Device ${index + 1}`,
            type: 'unknown', // We don't know type from DHCP
            status: 'online', // If in lease, usually active, but strictly means "has IP"
            blocked: false, // We'd need to check blocklist for this
            usage: '0 GB' // Per-device usage not easily available without extra tools
          }));
          setDevices(mappedDevices);
        }
      } catch (e) {
        console.error("Failed to fetch stats", e);
      }
    };

    fetchStats();
    const interval = setInterval(fetchStats, 30000); // Poll every 30 seconds
    return () => clearInterval(interval);
  }, []);

  // Load blocklist from router on page load
  useEffect(() => {
    const fetchBlocklist = async () => {
      try {
        const response = await fetch('/api/blocklist/custom');
        if (response.ok) {
          const data = await response.json();
          // data.blocklist is an array of domain strings
          const mappedBlocklist = data.blocklist.map((domain, index) => ({
            id: Date.now() + index, // Generate unique IDs
            domain: domain,
            active: true // All domains from router are active
          }));
          setCustomBlockList(mappedBlocklist);
        }
      } catch (e) {
        console.error("Failed to fetch blocklist", e);
      }
    };

    fetchBlocklist();
  }, []);

  // Blocklist Handlers
  const addCustomBlock = (domain) => {
    // Local-only add (no API call yet)
    setCustomBlockList(prev => [...prev, { id: Date.now(), domain, active: true }]);
  };

  const removeCustomBlock = (id) => {
    // Local-only remove (no API call yet)
    setCustomBlockList(prev => prev.filter(d => d.id !== id));
  };

  const toggleCustomBlock = (id) => {
    // Local-only toggle (no API call yet)
    setCustomBlockList(prev => prev.map(d => d.id === id ? { ...d, active: !d.active } : d));
  };

  const applyChanges = async (changes) => {
    // Apply all pending changes in a single batch
    const response = await fetch('/api/blocklist/apply', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ changes })
    });

    if (!response.ok) {
      throw new Error('Failed to apply changes');
    }
  };

  // Allowlist Handlers
  const addAllowDomain = async (domain) => {
    try {
      await fetch('/api/allow', {
        method: 'POST',
        headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
        body: `domain=${encodeURIComponent(domain)}`
      });
      setAllowList(prev => [...prev, { id: Date.now(), domain, active: true }]);
    } catch (e) {
      console.error("Failed to allow", e);
    }
  };
  const removeAllowDomain = (id) => {
    setAllowList(prev => prev.filter(d => d.id !== id));
  };
  const toggleAllowDomain = (id) => {
    setAllowList(prev => prev.map(d => d.id === id ? { ...d, active: !d.active } : d));
  };

  return (
    <div className="h-screen bg-slate-50 font-sans text-slate-900 flex flex-col max-w-md mx-auto relative shadow-2xl overflow-hidden selection:bg-indigo-100">
      <style>{`
        .hide-scrollbar::-webkit-scrollbar { display: none; }
        .hide-scrollbar { -ms-overflow-style: none; scrollbar-width: none; }
      `}</style>

      {/* Content Area */}
      <div className="flex-1 overflow-y-auto p-6 pb-32 scroll-smooth hide-scrollbar">
        {activeTab === 'overview' && (
          <Dashboard
            internetActive={internetActive}
            toggleInternet={toggleInternet}
            devices={devices}
            customBlockList={customBlockList}
            allowList={allowList}
            realStats={realStats}
            speed={speed}
          />
        )}
        {activeTab === 'allowlist' && (
          <AllowlistView
            allowList={allowList}
            addAllowDomain={addAllowDomain}
            removeAllowDomain={removeAllowDomain}
            toggleAllowDomain={toggleAllowDomain}
          />
        )}
        {activeTab === 'blocklist' && (
          <BlocklistView
            customBlockList={customBlockList}
            addCustomBlock={addCustomBlock}
            removeCustomBlock={removeCustomBlock}
            toggleCustomBlock={toggleCustomBlock}
            applyChanges={applyChanges}
          />
        )}
        {activeTab === 'devices' && (
          <DevicesList devices={devices} toggleDeviceBlock={toggleDeviceBlock} />
        )}
      </div>

      {/* Floating Bottom Nav */}
      <div className="absolute bottom-6 left-6 right-6">
        <div className="bg-white/90 backdrop-blur-xl rounded-2xl shadow-xl shadow-slate-200/50 border border-white/50 p-2 flex justify-between items-center ring-1 ring-slate-900/5">
          {[
            { id: 'overview', icon: <Home />, label: 'Overview' },
            { id: 'allowlist', icon: <ShieldCheck />, label: 'Allow' },
            { id: 'blocklist', icon: <Ban />, label: 'Block' },
            { id: 'devices', icon: <Smartphone />, label: 'Devices' },
          ].map((item) => (
            <button
              key={item.id}
              onClick={() => setActiveTab(item.id)}
              className={`flex-1 flex flex-col items-center justify-center py-3 rounded-xl transition-all duration-300 relative group
                ${activeTab === item.id ? 'text-indigo-600' : 'text-slate-400 hover:text-slate-600'}`}
            >
              <div className={`transform transition-all duration-300 ${activeTab === item.id ? '-translate-y-1' : ''}`}>
                {React.cloneElement(item.icon, {
                  size: 24,
                  strokeWidth: activeTab === item.id ? 2.5 : 2,
                  className: activeTab === item.id ? 'drop-shadow-sm' : ''
                })}
              </div>

              {/* Active Indicator Dot */}
              <div className={`absolute bottom-2 w-1 h-1 rounded-full bg-indigo-600 transition-all duration-300 
                ${activeTab === item.id ? 'scale-100 opacity-100' : 'scale-0 opacity-0'}`}
              />
            </button>
          ))}
        </div>
      </div>

    </div>
  );
};

export default App;
