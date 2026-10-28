import React from 'react';
import { TournamentFormat } from '../../types';
import { GitFork, Repeat } from 'lucide-react';

interface FormatBadgeProps {
  format: TournamentFormat;
}

export const FormatBadge: React.FC<FormatBadgeProps> = ({ format }) => {
  return (
    <span className="inline-flex items-center gap-1.5 px-2.5 py-0.5 rounded-full text-xs font-mono text-[#D7E2EA] bg-[#171a21] border border-[#D7E2EA]/15">
      {format === 'single_elimination' ? (
        <>
          <GitFork className="w-3 h-3 text-[#B600A8] rotate-90" />
          <span>Single Elimination</span>
        </>
      ) : (
        <>
          <Repeat className="w-3 h-3 text-[#7621B0]" />
          <span>Round Robin</span>
        </>
      )}
    </span>
  );
};
