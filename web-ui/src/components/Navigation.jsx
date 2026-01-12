import React from 'react';
import { LayoutDashboard, ShieldAlert, ShieldCheck } from "lucide-react";
import { cn } from "@/lib/utils";

const Navigation = ({ activeTab, setActiveTab }) => {
    const tabs = [
        { id: 'dashboard', label: 'Dashboard', icon: LayoutDashboard },
        { id: 'blocklist', label: 'Blocklist', icon: ShieldAlert },
        { id: 'allowlist', label: 'Allowlist', icon: ShieldCheck },
    ];

    return (
        <div className="fixed bottom-0 left-0 right-0 border-t bg-background/95 backdrop-blur supports-[backdrop-filter]:bg-background/60 z-50">
            <div className="flex justify-around items-center h-16 max-w-md mx-auto px-4">
                {tabs.map((tab) => {
                    const Icon = tab.icon;
                    const isActive = activeTab === tab.id;
                    return (
                        <button
                            key={tab.id}
                            onClick={() => setActiveTab(tab.id)}
                            className={cn(
                                "flex flex-col items-center justify-center w-full h-full space-y-1 transition-colors",
                                isActive
                                    ? "text-primary"
                                    : "text-muted-foreground hover:text-primary/80"
                            )}
                        >
                            <Icon className={cn("h-5 w-5", isActive && "fill-current")} />
                            <span className="text-[10px] font-medium">{tab.label}</span>
                        </button>
                    );
                })}
            </div>
        </div>
    );
};

export default Navigation;
