import React from 'react';
import { SchedulingConflict } from '../../types';
import { AlertTriangle } from 'lucide-react';

interface ConflictAlertProps {
  conflicts: SchedulingConflict[];
}

export const ConflictAlert: React.FC<ConflictAlertProps> = ({ conflicts }) => {
  if (conflicts.length === 0) return null;

  return (
    <div className="flex items-center gap-2 p-2.5 rounded-lg bg-amber-500/10 border border-amber-500/30 text-xs text-amber-300">
      <AlertTriangle className="w-4 h-4 text-amber-400 shrink-0" />
      <span>{conflicts[0].description}</span>
    </div>
  );
};
