import React, { useState } from 'react';
import { Card, CardContent } from "@/components/ui/card";
import { Button } from "@/components/ui/button";
import { Input } from "@/components/ui/input";
import { Plus, Trash2, ShieldCheck } from "lucide-react";
import {
    Dialog,
    DialogContent,
    DialogDescription,
    DialogFooter,
    DialogHeader,
    DialogTitle,
    DialogTrigger,
} from "@/components/ui/dialog";
import { ScrollArea } from "@/components/ui/scroll-area";

const Allowlist = () => {
    const [allowedItems, setAllowedItems] = useState([
        { id: 1, domain: "work-resources.com", added: "2023-10-27" },
        { id: 2, domain: "school-portal.edu", added: "2023-10-28" },
    ]);
    const [newItem, setNewItem] = useState("");
    const [isOpen, setIsOpen] = useState(false);

    const handleAdd = () => {
        if (newItem) {
            setAllowedItems([...allowedItems, { id: Date.now(), domain: newItem, added: new Date().toISOString().split('T')[0] }]);
            setNewItem("");
            setIsOpen(false);
        }
    };

    const handleDelete = (id) => {
        setAllowedItems(allowedItems.filter(item => item.id !== id));
    };

    return (
        <div className="space-y-4 p-4 pb-20">
            <div className="flex items-center justify-between">
                <h1 className="text-2xl font-bold tracking-tight">Allowlist</h1>
                <Dialog open={isOpen} onOpenChange={setIsOpen}>
                    <DialogTrigger asChild>
                        <Button size="sm">
                            <Plus className="h-4 w-4 mr-2" /> Add
                        </Button>
                    </DialogTrigger>
                    <DialogContent className="sm:max-w-[425px]">
                        <DialogHeader>
                            <DialogTitle>Add to Allowlist</DialogTitle>
                            <DialogDescription>
                                Enter the domain you want to allow.
                            </DialogDescription>
                        </DialogHeader>
                        <div className="grid gap-4 py-4">
                            <Input
                                id="domain"
                                placeholder="example.com"
                                value={newItem}
                                onChange={(e) => setNewItem(e.target.value)}
                            />
                        </div>
                        <DialogFooter>
                            <Button onClick={handleAdd}>Allow Domain</Button>
                        </DialogFooter>
                    </DialogContent>
                </Dialog>
            </div>

            <ScrollArea className="h-[calc(100vh-180px)]">
                <div className="space-y-2">
                    {allowedItems.map((item) => (
                        <Card key={item.id} className="overflow-hidden">
                            <CardContent className="p-4 flex items-center justify-between">
                                <div className="flex items-center gap-3">
                                    <div className="h-8 w-8 rounded-full bg-green-100 dark:bg-green-900/20 flex items-center justify-center">
                                        <ShieldCheck className="h-4 w-4 text-green-600 dark:text-green-400" />
                                    </div>
                                    <div>
                                        <p className="font-medium">{item.domain}</p>
                                        <p className="text-xs text-muted-foreground">Added: {item.added}</p>
                                    </div>
                                </div>
                                <Button variant="ghost" size="icon" onClick={() => handleDelete(item.id)}>
                                    <Trash2 className="h-4 w-4 text-muted-foreground hover:text-red-500" />
                                </Button>
                            </CardContent>
                        </Card>
                    ))}
                    {allowedItems.length === 0 && (
                        <div className="text-center p-8 text-muted-foreground">
                            No allowed domains.
                        </div>
                    )}
                </div>
            </ScrollArea>
        </div>
    );
};

export default Allowlist;
