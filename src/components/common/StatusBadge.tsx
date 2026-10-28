import React from 'react';
import { CompetitionStatus, MatchStatus } from '../../types';

interface StatusBadgeProps {
  status: CompetitionStatus | MatchStatus;
  size?: 'sm' | 'md';
}

export const StatusBadge: React.FC<StatusBadgeProps> = ({ status, size = 'md' }) => {
  const sizeClasses = size === 'sm' ? 'text-[10px] px-2 py-0.5' : 'text-xs px-2.5 py-0.5';

  switch (status) {
    case 'live':
      return (
        <span
          className={`inline-flex items-center gap-1.5 font-medium rounded-full font-mono uppercase tracking-wider bg-[#B600A8]/20 text-white border border-[#B600A8]/50 ${sizeClasses}`}
        >
          <span className="w-1.5 h-1.5 rounded-full bg-[#B600A8] animate-pulse" />
          Live
        </span>
      );
    case 'completed':
      return (
        <span
          className={`inline-flex items-center font-medium rounded-full font-mono uppercase tracking-wider bg-[#171a21] text-[#D7E2EA]/70 border border-[#D7E2EA]/15 ${sizeClasses}`}
        >
          Completed
        </span>
      );
    case 'upcoming':
    default:
      return (
        <span
          className={`inline-flex items-center font-medium rounded-full font-mono uppercase tracking-wider bg-[#18011F] text-[#D7E2EA] border border-[#7621B0]/40 ${sizeClasses}`}
        >
          Upcoming
        </span>
      );
  }
};
