import React from 'react';
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card";
import { Activity, Wifi, ArrowDown, ArrowUp } from "lucide-react";

const Dashboard = () => {
    // Mock data
    const stats = {
        connectedDevices: 5,
        dataUsage: {
            total: "1.2 GB",
            download: "800 MB",
            upload: "400 MB"
        }
    };

    return (
        <div className="space-y-4 p-4 pb-20">
            <h1 className="text-2xl font-bold tracking-tight">Dashboard</h1>

            <div className="grid gap-4 md:grid-cols-2">
                <Card>
                    <CardHeader className="flex flex-row items-center justify-between space-y-0 pb-2">
                        <CardTitle className="text-sm font-medium">
                            Connected Devices
                        </CardTitle>
                        <Wifi className="h-4 w-4 text-muted-foreground" />
                    </CardHeader>
                    <CardContent>
                        <div className="text-2xl font-bold">{stats.connectedDevices}</div>
                        <p className="text-xs text-muted-foreground">
                            Active connections
                        </p>
                    </CardContent>
                </Card>

                <Card>
                    <CardHeader className="flex flex-row items-center justify-between space-y-0 pb-2">
                        <CardTitle className="text-sm font-medium">
                            Data Usage
                        </CardTitle>
                        <Activity className="h-4 w-4 text-muted-foreground" />
                    </CardHeader>
                    <CardContent>
                        <div className="text-2xl font-bold">{stats.dataUsage.total}</div>
                        <div className="flex items-center gap-4 mt-2">
                            <div className="flex items-center text-xs text-green-500">
                                <ArrowDown className="h-3 w-3 mr-1" />
                                {stats.dataUsage.download}
                            </div>
                            <div className="flex items-center text-xs text-blue-500">
                                <ArrowUp className="h-3 w-3 mr-1" />
                                {stats.dataUsage.upload}
                            </div>
                        </div>
                    </CardContent>
                </Card>
            </div>
        </div>
    );
};

export default Dashboard;
