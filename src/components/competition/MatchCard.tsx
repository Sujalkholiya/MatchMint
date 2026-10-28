import React from 'react';
import { Match, Team } from '../../types';
import { StatusBadge } from '../common/StatusBadge';
import { Clock, MapPin, Edit3, CheckCircle, Trophy } from 'lucide-react';

interface MatchCardProps {
  match: Match;
  teams: Team[];
  onOpenResultModal: (match: Match) => void;
  onOpenScheduleModal?: (match: Match) => void;
  compact?: boolean;
}

export const MatchCard: React.FC<MatchCardProps> = ({
  match,
  teams,
  onOpenResultModal,
  onOpenScheduleModal,
  compact = false,
}) => {
  const team1 = teams.find((t) => t.id === match.team1Id);
  const team2 = teams.find((t) => t.id === match.team2Id);

  const team1Won = match.status === 'completed' && match.winnerId === match.team1Id;
  const team2Won = match.status === 'completed' && match.winnerId === match.team2Id;

  return (
    <div
      className={`group relative rounded-2xl border transition-all duration-150 bg-[#12151a] hover:bg-[#171a21] ${
        match.status === 'completed'
          ? 'border-[#D7E2EA]/15'
          : 'border-[#D7E2EA]/15 hover:border-[#B600A8]/40'
      } ${compact ? 'p-3' : 'p-4'}`}
    >
      {/* Top Metadata Header: Round Name, Status, Venue/Time */}
      <div className="flex items-center justify-between gap-2 mb-3 pb-2.5 border-b border-[#D7E2EA]/10 text-xs">
        <div className="flex items-center gap-2">
          <span className="font-mono font-medium text-[#D7E2EA]/60">
            #{match.matchNumber}
          </span>
          <span className="font-semibold text-white">
            {match.roundName}
          </span>
        </div>
        <StatusBadge status={match.status} size="sm" />
      </div>

      {/* Match Competitors & Scores */}
      <div className="space-y-2 mb-3">
        {/* Team 1 */}
        <div
          className={`flex items-center justify-between px-2.5 py-1.5 rounded-xl transition-colors ${
            team1Won
              ? 'bg-[#B600A8]/20 text-white font-bold'
              : match.status === 'completed'
              ? 'text-[#D7E2EA]/40 font-normal'
              : 'text-white font-medium'
          }`}
        >
          <div className="flex items-center gap-2 min-w-0">
            {team1 ? (
              <>
                <span className="text-[10px] font-mono font-medium px-1.5 py-0.5 rounded bg-[#171a21] text-[#D7E2EA]/60">
                  #{team1.seed}
                </span>
                <span className="truncate text-sm tracking-tight">{team1.name}</span>
                {team1Won && <Trophy className="w-3.5 h-3.5 text-[#B600A8] shrink-0" />}
              </>
            ) : (
              <span className="text-xs italic text-[#D7E2EA]/40">TBD (Pending Previous Round)</span>
            )}
          </div>
          <div className="font-mono text-base tabular-nums font-bold ml-2">
            {match.team1Score !== null ? (
              <span className={team1Won ? 'text-[#B600A8]' : 'text-[#D7E2EA]/70'}>
                {match.team1Score}
              </span>
            ) : (
              <span className="text-[#D7E2EA]/30">—</span>
            )}
          </div>
        </div>

        {/* Team 2 */}
        <div
          className={`flex items-center justify-between px-2.5 py-1.5 rounded-xl transition-colors ${
            team2Won
              ? 'bg-[#B600A8]/20 text-white font-bold'
              : match.status === 'completed'
              ? 'text-[#D7E2EA]/40 font-normal'
              : 'text-white font-medium'
          }`}
        >
          <div className="flex items-center gap-2 min-w-0">
            {team2 ? (
              <>
                <span className="text-[10px] font-mono font-medium px-1.5 py-0.5 rounded bg-[#171a21] text-[#D7E2EA]/60">
                  #{team2.seed}
                </span>
                <span className="truncate text-sm tracking-tight">{team2.name}</span>
                {team2Won && <Trophy className="w-3.5 h-3.5 text-[#B600A8] shrink-0" />}
              </>
            ) : (
              <span className="text-xs italic text-[#D7E2EA]/40">TBD (Pending Previous Round)</span>
            )}
          </div>
          <div className="font-mono text-base tabular-nums font-bold ml-2">
            {match.team2Score !== null ? (
              <span className={team2Won ? 'text-[#B600A8]' : 'text-[#D7E2EA]/70'}>
                {match.team2Score}
              </span>
            ) : (
              <span className="text-[#D7E2EA]/30">—</span>
            )}
          </div>
        </div>
      </div>

      {/* Tie-breaker note if any */}
      {match.tieBreakerDetails && (
        <div className="mb-3 px-2.5 py-1 rounded-xl bg-[#0C0C0C] text-[11px] font-mono text-[#D7E2EA]/80 border border-[#B600A8]/30">
          Decider: {match.tieBreakerDetails}
        </div>
      )}

      {/* Footer: Venue, Schedule Time & CTA Button */}
      <div className="flex items-center justify-between pt-2.5 border-t border-[#D7E2EA]/10 text-xs text-[#D7E2EA]/60">
        <div className="flex items-center gap-3">
          <div className="flex items-center gap-1.5">
            <Clock className="w-3.5 h-3.5 text-[#D7E2EA]/40" />
            <span className="font-mono text-[11px]">{match.scheduledTime}</span>
          </div>
          <div className="hidden sm:flex items-center gap-1.5">
            <MapPin className="w-3.5 h-3.5 text-[#D7E2EA]/40" />
            <span className="truncate max-w-[120px]">{match.venue}</span>
          </div>
        </div>

        <div className="flex items-center gap-2">
          {onOpenScheduleModal && match.status !== 'completed' && (
            <button
              onClick={() => onOpenScheduleModal(match)}
              className="text-[11px] text-[#D7E2EA]/60 hover:text-white px-2 py-1 rounded-lg hover:bg-[#171a21] transition-colors cursor-pointer"
              title="Reschedule venue or time"
            >
              Reschedule
            </button>
          )}

          <button
            onClick={() => onOpenResultModal(match)}
            disabled={!match.team1Id || !match.team2Id}
            style={
              match.status !== 'completed' && match.team1Id && match.team2Id
                ? {
                    background:
                      'linear-gradient(123deg, #18011F 7%, #B600A8 37%, #7621B0 72%, #BE4C00 100%)',
                    boxShadow:
                      '0px 2px 4px rgba(181, 1, 167, 0.25), 2px 2px 8px #7721B1 inset',
                    outline: '1px solid #ffffff',
                    outlineOffset: '-2px',
                  }
                : undefined
            }
            className={`inline-flex items-center gap-1.5 px-3 py-1 text-xs font-medium uppercase tracking-wider rounded-full transition-all cursor-pointer ${
              !match.team1Id || !match.team2Id
                ? 'opacity-30 cursor-not-allowed bg-[#171a21] text-[#D7E2EA]/40'
                : match.status === 'completed'
                ? 'border border-[#D7E2EA]/20 bg-[#171a21] text-[#D7E2EA] hover:bg-[#D7E2EA]/10'
                : 'text-white hover:opacity-90'
            }`}
          >
            {match.status === 'completed' ? (
              <>
                <Edit3 className="w-3 h-3" />
                <span>Edit Score</span>
              </>
            ) : (
              <>
                <CheckCircle className="w-3 h-3" />
                <span>Enter Result</span>
              </>
            )}
          </button>
        </div>
      </div>
    </div>
  );
};
