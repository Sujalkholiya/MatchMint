import React from 'react';
import { Competition, Match, Team } from '../../types';
import { StatusBadge } from '../common/StatusBadge';
import { FormatBadge } from '../common/FormatBadge';
import { ArrowRight, Users } from 'lucide-react';

interface CompetitionCardProps {
  competition: Competition;
  teams: Team[];
  matches: Match[];
  onSelect: (comp: Competition) => void;
}

export const CompetitionCard: React.FC<CompetitionCardProps> = ({
  competition,
  teams,
  matches,
  onSelect,
}) => {
  const compTeams = teams.filter((t) => t.competitionId === competition.id);
  const compMatches = matches.filter((m) => m.competitionId === competition.id);
  const completedCount = compMatches.filter((m) => m.status === 'completed').length;

  return (
    <div
      onClick={() => onSelect(competition)}
      className="p-5 rounded-2xl border border-[#D7E2EA]/15 bg-[#12151a] hover:bg-[#171a21] hover:border-[#B600A8]/40 transition-all cursor-pointer flex flex-col justify-between space-y-4 shadow-sm group"
    >
      <div className="space-y-2.5">
        {/* At most 2 badges per card (status + format) */}
        <div className="flex items-center justify-between">
          <StatusBadge status={competition.status} size="sm" />
          <FormatBadge format={competition.format} />
        </div>

        <div>
          <h3 className="text-base font-bold text-white group-hover:text-[#D7E2EA] transition-colors">
            {competition.name}
          </h3>
          <p className="text-xs text-[#D7E2EA]/60 mt-1 line-clamp-2">
            {competition.description}
          </p>
        </div>
      </div>

      <div className="pt-3 border-t border-[#D7E2EA]/10 flex items-center justify-between text-xs text-[#D7E2EA]/60">
        <span className="flex items-center gap-1.5 font-mono text-[11px]">
          <Users className="w-3.5 h-3.5 text-[#D7E2EA]/40" />
          <span>{compTeams.length} / {competition.totalTeams} Teams</span>
        </span>

        <span className="font-mono text-[11px] text-[#D7E2EA]/80 tabular-nums">
          {completedCount} / {compMatches.length} Matches
        </span>

        <span className="text-[#B600A8] flex items-center gap-1 font-medium group-hover:translate-x-0.5 transition-transform">
          <span>View</span>
          <ArrowRight className="w-3.5 h-3.5" />
        </span>
      </div>
    </div>
  );
};
