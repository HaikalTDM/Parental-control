import React, { useState } from 'react';
import {
    Wifi, WifiOff, Smartphone, Shield, Clock, Home, Activity,
    Zap, Lock, Unlock, PlayCircle, Youtube, Facebook,
    Gamepad2, Instagram, Check, Plus, Trash2, Ban, Globe, ShieldCheck,
    ArrowUp, ArrowDown, Users, ShieldAlert, Terminal, ChevronDown, ChevronUp
} from 'lucide-react';

// UI Components

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

// View Components

const StatusRing = ({ internetActive, toggleInternet }) => (
    <div className="relative flex justify-center items-center py-6">
        <div className={`absolute w-64 h-64 rounded-full opacity-10 transition-colors duration-500 ${internetActive ? 'bg-emerald-400 animate-pulse' : 'bg-rose-400'}`}></div>
        <div className={`absolute w-52 h-52 rounded-full opacity-20 transition-colors duration-500 ${internetActive ? 'bg-emerald-400' : 'bg-rose-400'}`}></div>

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
            {subValue && <p className="text-[10px] text-slate-400 mt-0.5">{subValue}</p>}
        </div>
    </Card>
);

const LogViewer = ({ logs, isLoading, expanded, toggleExpanded }) => {
    const logContainerRef = React.useRef(null);

    // Auto-scroll to bottom when new logs arrive
    React.useEffect(() => {
        if (logContainerRef.current && expanded) {
            logContainerRef.current.scrollTop = logContainerRef.current.scrollHeight;
        }
    }, [logs, expanded]);

    const getLogLevelColor = (level) => {
        switch (level) {
            case 'error':
                return 'text-rose-600 bg-rose-50';
            case 'success':
                return 'text-emerald-600 bg-emerald-50';
            default:
                return 'text-blue-600 bg-blue-50';
        }
    };

    return (
        <Card className="overflow-hidden">
            <button
                onClick={toggleExpanded}
                className="w-full p-4 flex items-center justify-between hover:bg-slate-50 transition-colors"
            >
                <div className="flex items-center gap-3">
                    <div className="p-2 bg-slate-100 rounded-lg text-slate-600">
                        <Terminal className="w-5 h-5" />
                    </div>
                    <div className="text-left">
                        <p className="text-sm font-bold text-slate-700">System Logs</p>
                        <p className="text-xs text-slate-400">
                            {isLoading ? 'Updating blocklist...' : `${logs.length} entries`}
                        </p>
                    </div>
                </div>
                <div className="flex items-center gap-2">
                    {isLoading && (
                        <div className="w-2 h-2 bg-amber-500 rounded-full animate-pulse"></div>
                    )}
                    {expanded ? <ChevronUp className="w-5 h-5 text-slate-400" /> : <ChevronDown className="w-5 h-5 text-slate-400" />}
                </div>
            </button>

            {expanded && (
                <div className="border-t border-slate-200">
                    <div
                        ref={logContainerRef}
                        className="max-h-72 overflow-y-auto p-3 space-y-2 bg-slate-50/50"
                    >
                        {logs.length === 0 ? (
                            <div className="text-center py-8 text-slate-400 text-sm">
                                No logs available yet
                            </div>
                        ) : (
                            logs.map((log, index) => (
                                <div
                                    key={index}
                                    className={`p-2.5 rounded-lg border ${getLogLevelColor(log.level)} border-opacity-50`}
                                >
                                    <div className="flex items-start gap-2">
                                        <div className="flex-1">
                                            <p className="text-xs font-mono text-slate-500 mb-1">
                                                {log.timestamp}
                                            </p>
                                            <p className="text-xs text-slate-700 leading-relaxed">
                                                {log.message}
                                            </p>
                                        </div>
                                    </div>
                                </div>
                            ))
                        )}
                    </div>
                </div>
            )}
        </Card>
    );
};

const Dashboard = ({ internetActive, toggleInternet, devices, blockedApps, customBlockList, allowList, adblockStatus, adblockLogs, logsExpanded, toggleLogsExpanded }) => {
    const onlineDevices = devices.filter(d => d.status === 'online').length;
    const totalAppsBlocked = Object.values(blockedApps).filter(Boolean).length;
    const activeCustomBlocks = customBlockList.filter(d => d.active).length;
    const totalBlocked = totalAppsBlocked + activeCustomBlocks;
    const totalAllowed = allowList.filter(d => d.active).length;

    // Determine status display
    const getStatusInfo = () => {
        if (!internetActive) {
            return { text: 'Internet Paused', color: 'bg-rose-500', pulse: false };
        }
        if (adblockStatus === 'loading') {
            return { text: 'Loading Blocklist...', color: 'bg-amber-500', pulse: true };
        }
        if (adblockStatus === 'error') {
            return { text: 'Adblock Error', color: 'bg-rose-500', pulse: true };
        }
        return { text: 'System Operational', color: 'bg-emerald-500', pulse: true };
    };

    const statusInfo = getStatusInfo();

    return (
        <div className="space-y-6 animate-in fade-in slide-in-from-bottom-4 duration-500">

            <div className="flex justify-between items-center px-1">
                <div>
                    <h1 className="text-2xl font-bold text-slate-900">System Overview</h1>
                    <div className="flex items-center gap-2 mt-1">
                        <span className={`w-2 h-2 rounded-full ${statusInfo.color} ${statusInfo.pulse ? 'animate-pulse' : ''}`}></span>
                        <p className="text-sm text-slate-500 font-medium">{statusInfo.text}</p>
                    </div>
                </div>
                <div className="w-10 h-10 rounded-full bg-slate-100 flex items-center justify-center border border-slate-200">
                    <Activity className="w-5 h-5 text-slate-500" />
                </div>
            </div>

            <StatusRing internetActive={internetActive} toggleInternet={toggleInternet} />

            <div className="grid grid-cols-2 gap-3">
                <StatCard
                    icon={Users}
                    label="Online"
                    value={onlineDevices}
                    subValue={`of ${devices.length} Devices`}
                    colorClass="text-blue-600"
                    bgClass="bg-blue-50"
                />

                <StatCard
                    icon={ShieldAlert}
                    label="Blocking"
                    value={totalBlocked}
                    subValue={`${totalAppsBlocked} Apps, ${activeCustomBlocks} Sites`}
                    colorClass="text-rose-600"
                    bgClass="bg-rose-50"
                />

                <StatCard
                    icon={ShieldCheck}
                    label="Allowlist"
                    value={totalAllowed}
                    subValue="Safe Domains"
                    colorClass="text-emerald-600"
                    bgClass="bg-emerald-50"
                />

                <StatCard
                    icon={Zap}
                    label="Speed"
                    value="45"
                    subValue="Mbps Avg."
                    colorClass="text-amber-600"
                    bgClass="bg-amber-50"
                />
            </div>

            <LogViewer
                logs={adblockLogs}
                isLoading={adblockStatus === 'loading'}
                expanded={logsExpanded}
                toggleExpanded={toggleLogsExpanded}
            />

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
                    <p className="text-lg font-bold text-slate-800">4.2 GB</p>
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

const BlocklistView = ({ blockedApps, toggleAppBlock, customBlockList, addCustomBlock, removeCustomBlock, toggleCustomBlock }) => {
    const [newDomain, setNewDomain] = useState('');

    const handleAdd = () => {
        if (newDomain) {
            addCustomBlock(newDomain);
            setNewDomain('');
        }
    }

    const apps = [
        { id: 'youtube', name: 'YouTube', icon: <Youtube />, color: 'text-red-600 bg-red-50' },
        { id: 'tiktok', name: 'TikTok', icon: <PlayCircle />, color: 'text-pink-600 bg-pink-50' },
        { id: 'facebook', name: 'Facebook', icon: <Facebook />, color: 'text-blue-600 bg-blue-50' },
        { id: 'roblox', name: 'Roblox', icon: <Gamepad2 />, color: 'text-orange-600 bg-orange-50' },
        { id: 'instagram', name: 'Instagram', icon: <Instagram />, color: 'text-purple-600 bg-purple-50' },
    ];

    return (
        <div className="space-y-6 animate-in fade-in slide-in-from-bottom-4 duration-500">
            <div className="px-1">
                <h1 className="text-2xl font-bold text-slate-900">Edit Blocklist</h1>
                <p className="text-sm text-slate-500 font-medium">Prevent access to these sites</p>
            </div>

            <div className="space-y-3">
                <h3 className="text-xs font-bold uppercase text-slate-400 tracking-wider px-1">Popular Apps</h3>
                {apps.map((app) => (
                    <Card key={app.id} className="p-3 flex items-center justify-between">
                        <div className="flex items-center gap-4">
                            <div className={`w-10 h-10 rounded-lg flex items-center justify-center transition-opacity duration-200 ${blockedApps[app.id] ? 'opacity-50 grayscale' : 'opacity-100'} ${app.color}`}>
                                {React.cloneElement(app.icon, { size: 20 })}
                            </div>
                            <span className={`font-bold text-sm transition-colors duration-200 ${blockedApps[app.id] ? 'text-slate-400' : 'text-slate-700'}`}>
                                {app.name}
                            </span>
                        </div>
                        <Switch
                            checked={blockedApps[app.id]}
                            onCheckedChange={() => toggleAppBlock(app.id)}
                            colorClass="bg-rose-500"
                        />
                    </Card>
                ))}
            </div>

            <div className="space-y-3">
                <h3 className="text-xs font-bold uppercase text-slate-400 tracking-wider px-1">Custom Domains</h3>

                <div className="flex gap-2">
                    <input
                        type="text"
                        value={newDomain}
                        onChange={(e) => setNewDomain(e.target.value)}
                        placeholder="e.g. gambling.com"
                        className="flex-1 bg-white border border-slate-200 rounded-xl px-4 py-3 text-sm focus:outline-none focus:ring-2 focus:ring-rose-500 focus:border-transparent shadow-sm"
                    />
                    <button
                        onClick={handleAdd}
                        className="bg-rose-500 text-white p-3 rounded-xl shadow-lg shadow-rose-200 active:scale-95 transition-transform"
                    >
                        <Plus className="w-5 h-5" />
                    </button>
                </div>

                {customBlockList.map((item) => (
                    <div key={item.id} className={`bg-white border rounded-xl p-3 flex justify-between items-center shadow-sm transition-all ${item.active ? 'border-rose-200 bg-rose-50/10' : 'border-slate-200'}`}>
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
                                onClick={() => removeCustomBlock(item.id)}
                                className="text-slate-300 hover:text-rose-500 p-2 transition-colors"
                                title="Delete Rule"
                            >
                                <Trash2 className="w-4 h-4" />
                            </button>

                            <Switch
                                checked={item.active}
                                onCheckedChange={() => toggleCustomBlock(item.id)}
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

// Main App

const App = () => {
    const [activeTab, setActiveTab] = useState('overview');
    const [internetActive, setInternetActive] = useState(true);
    const [devices, setDevices] = useState([]);
    const [blockedApps, setBlockedApps] = useState({});
    const [customBlockList, setCustomBlockList] = useState([]);
    const [allowList, setAllowList] = useState([]);
    const [loading, setLoading] = useState(true);
    const [adblockStatus, setAdblockStatus] = useState('idle'); // idle, loading, error
    const [adblockLogs, setAdblockLogs] = useState([]);
    const [logsExpanded, setLogsExpanded] = useState(false);

    const API_URL = import.meta.env.DEV ? 'http://localhost:5050/api' : '/api';

    // Fetch Data
    React.useEffect(() => {
        const fetchData = async () => {
            try {
                const [statusRes, devicesRes, blocklistRes, allowlistRes, adblockStatusRes] = await Promise.all([
                    fetch(`${API_URL}/status`),
                    fetch(`${API_URL}/devices`),
                    fetch(`${API_URL}/blocklist`),
                    fetch(`${API_URL}/allowlist`),
                    fetch(`${API_URL}/adblock/status`)
                ]);

                const statusData = await statusRes.json();
                const devicesData = await devicesRes.json();
                const blocklistData = await blocklistRes.json();
                const allowlistData = await allowlistRes.json();
                const adblockStatusData = await adblockStatusRes.json();

                setInternetActive(statusData.internet_active);
                setDevices(devicesData);
                setBlockedApps(blocklistData.apps);
                setCustomBlockList(blocklistData.custom);
                setAllowList(allowlistData);
                setAdblockStatus(adblockStatusData.status);
                setLoading(false);
            } catch (error) {
                console.error("Error fetching data:", error);
                setLoading(false);
            }
        };

        fetchData();
    }, []);

    // Poll adblock status when loading
    React.useEffect(() => {
        if (adblockStatus !== 'loading') return;

        const interval = setInterval(async () => {
            try {
                const res = await fetch(`${API_URL}/adblock/status`);
                const data = await res.json();
                setAdblockStatus(data.status);
            } catch (error) {
                console.error("Error polling adblock status:", error);
            }
        }, 2000);

        return () => clearInterval(interval);
    }, [adblockStatus, API_URL]);

    // Fetch logs when expanded or status changes
    React.useEffect(() => {
        if (!logsExpanded && adblockStatus === 'idle') return;

        const fetchLogs = async () => {
            try {
                const res = await fetch(`${API_URL}/adblock/logs`);
                const logs = await res.json();
                setAdblockLogs(logs);
            } catch (error) {
                console.error("Error fetching logs:", error);
            }
        };

        fetchLogs();

        // Auto-refresh logs while loading
        if (adblockStatus === 'loading') {
            const interval = setInterval(fetchLogs, 2000);
            return () => clearInterval(interval);
        }
    }, [logsExpanded, adblockStatus, API_URL]);

    // Event Handlers
    const toggleInternet = async () => {
        try {
            const res = await fetch(`${API_URL}/toggle-internet`, { method: 'POST' });
            const data = await res.json();
            setInternetActive(data.internet_active);
        } catch (error) {
            console.error("Error toggling internet:", error);
        }
    };

    const toggleDeviceBlock = async (id) => {
        try {
            const res = await fetch(`${API_URL}/device/${id}/block`, { method: 'POST' });
            const data = await res.json();
            setDevices(data);
        } catch (error) {
            console.error("Error toggling device block:", error);
        }
    };

    const toggleAppBlock = async (appId) => {
        try {
            const res = await fetch(`${API_URL}/blocklist/app`, {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ id: appId })
            });
            const data = await res.json();
            setBlockedApps(data);
        } catch (error) {
            console.error("Error toggling app block:", error);
        }
    };

    const addCustomBlock = async (domain) => {
        try {
            const res = await fetch(`${API_URL}/blocklist/custom`, {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ domain })
            });
            const data = await res.json();
            setCustomBlockList(data);
        } catch (error) {
            console.error("Error adding custom block:", error);
        }
    };

    const removeCustomBlock = async (id) => {
        try {
            const res = await fetch(`${API_URL}/blocklist/custom/${id}`, { method: 'DELETE' });
            const data = await res.json();
            setCustomBlockList(data);
        } catch (error) {
            console.error("Error removing custom block:", error);
        }
    };

    const toggleCustomBlock = async (id) => {
        try {
            const res = await fetch(`${API_URL}/blocklist/custom/${id}/toggle`, { method: 'POST' });
            const data = await res.json();
            setCustomBlockList(data);
        } catch (error) {
            console.error("Error toggling custom block:", error);
        }
    };

    const addAllowDomain = async (domain) => {
        try {
            const res = await fetch(`${API_URL}/allowlist`, {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ domain })
            });
            const data = await res.json();
            setAllowList(data);
        } catch (error) {
            console.error("Error adding allow domain:", error);
        }
    };

    const removeAllowDomain = async (id) => {
        try {
            const res = await fetch(`${API_URL}/allowlist/${id}`, { method: 'DELETE' });
            const data = await res.json();
            setAllowList(data);
        } catch (error) {
            console.error("Error removing allow domain:", error);
        }
    };

    const toggleAllowDomain = async (id) => {
        try {
            const res = await fetch(`${API_URL}/allowlist/${id}/toggle`, { method: 'POST' });
            const data = await res.json();
            setAllowList(data);
        } catch (error) {
            console.error("Error toggling allow domain:", error);
        }
    };

    if (loading) {
        return (
            <div className="h-screen flex items-center justify-center bg-slate-50 text-slate-400">
                <Activity className="w-8 h-8 animate-spin" />
            </div>
        );
    }

    return (
        <div className="h-screen bg-slate-50 font-sans text-slate-900 flex flex-col max-w-md mx-auto relative shadow-2xl overflow-hidden selection:bg-indigo-100">
            <style>{`
        .hide-scrollbar::-webkit-scrollbar { display: none; }
        .hide-scrollbar { -ms-overflow-style: none; scrollbar-width: none; }
      `}</style>

            <div className="flex-1 overflow-y-auto p-6 pb-32 scroll-smooth hide-scrollbar">
                {activeTab === 'overview' && (
                    <Dashboard
                        internetActive={internetActive}
                        toggleInternet={toggleInternet}
                        devices={devices}
                        blockedApps={blockedApps}
                        customBlockList={customBlockList}
                        allowList={allowList}
                        adblockStatus={adblockStatus}
                        adblockLogs={adblockLogs}
                        logsExpanded={logsExpanded}
                        toggleLogsExpanded={() => setLogsExpanded(!logsExpanded)}
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
                        blockedApps={blockedApps}
                        toggleAppBlock={toggleAppBlock}
                        customBlockList={customBlockList}
                        addCustomBlock={addCustomBlock}
                        removeCustomBlock={removeCustomBlock}
                        toggleCustomBlock={toggleCustomBlock}
                    />
                )}
                {activeTab === 'devices' && (
                    <DevicesList devices={devices} toggleDeviceBlock={toggleDeviceBlock} />
                )}
            </div>

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
