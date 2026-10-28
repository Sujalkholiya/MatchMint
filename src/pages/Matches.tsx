import React, { useState } from 'react';
import { Match, Team, Competition, HistoryRecord } from '../types';
import { Modal } from '../components/common/Modal';
import { EmptyState } from '../components/common/EmptyState';
import { FadeIn } from '../components/landing/FadeIn';
import {
  Calendar,
  RotateCcw,
  ArrowRight,
} from 'lucide-react';

interface MatchesProps {
  matches: Match[];
  teams: Team[];
  competitions: Competition[];
  history?: HistoryRecord[];
  onOpenResultModal: (match: Match) => void;
  onOpenScheduleModal?: (match: Match) => void;
  onUndoResult?: (historyId: string) => void;
}

export const Matches: React.FC<MatchesProps> = ({
  matches,
  teams,
  competitions,
  history = [],
  onOpenResultModal,
  onUndoResult,
}) => {
  const [activeTab, setActiveTab] = useState<'upcoming' | 'completed'>('upcoming');
  const [selectedCompFilter, setSelectedCompFilter] = useState<string>('all');
  const [selectedRoundFilter, setSelectedRoundFilter] = useState<string>('all');
  const [undoTargetRecord, setUndoTargetRecord] = useState<HistoryRecord | null>(null);

  // Extract all rounds
  const allRounds = Array.from(new Set(matches.map((m) => m.roundName))).filter(Boolean);

  // Find the latest undoable history record
  const latestUndoable = history.find(
    (h) => h.canUndo && (h.actionType === 'RESULT_ENTERED' || h.matchId)
  );

  // Filter matches
  const filteredMatches = matches.filter((m) => {
    if (activeTab === 'upcoming' && m.status !== 'upcoming') return false;
    if (activeTab === 'completed' && m.status !== 'completed') return false;

    if (selectedCompFilter !== 'all' && m.competitionId !== selectedCompFilter) return false;
    if (selectedRoundFilter !== 'all' && m.roundName !== selectedRoundFilter) return false;

    return true;
  });

  const getTeamName = (teamId: string | null, fallback: string) => {
    if (!teamId) return fallback;
    const t = teams.find((team) => team.id === teamId);
    return t ? t.name : fallback;
  };

  // Identify the most recently completed match (to attach undo button)
  const mostRecentCompletedMatchId = latestUndoable?.matchId || null;

  const handleConfirmUndo = () => {
    if (undoTargetRecord && onUndoResult) {
      onUndoResult(undoTargetRecord.id);
      setUndoTargetRecord(null);
    }
  };

  return (
    <div className="p-6 max-w-6xl mx-auto space-y-6">
      {/* Header */}
      <FadeIn delay={0} y={20}>
        <div className="flex flex-col sm:flex-row sm:items-center justify-between gap-4 pb-2">
          <div>
            <h1 className="hero-heading text-3xl md:text-5xl font-black uppercase tracking-tight">
              Matches
            </h1>
            <p className="text-xs text-[#D7E2EA]/60 mt-1">
              Tournament fixtures, schedule operations, and live results.
            </p>
          </div>

          {/* Global subtle Undo for latest result if available */}
          {latestUndoable && (
            <button
              onClick={() => setUndoTargetRecord(latestUndoable)}
              className="inline-flex items-center gap-1.5 px-3.5 py-1.5 rounded-full text-xs font-mono uppercase tracking-wider text-[#D7E2EA] hover:bg-[#B600A8]/20 border border-[#B600A8]/40 transition-colors cursor-pointer self-start sm:self-auto"
              title="Result Recovery: Revert most recent match result"
            >
              <RotateCcw className="w-3.5 h-3.5 text-[#B600A8]" />
              <span>Undo last result</span>
            </button>
          )}
        </div>
      </FadeIn>

      {/* Tabs and Filters */}
      <FadeIn delay={0.05} y={20}>
        <div className="flex flex-col sm:flex-row sm:items-center justify-between gap-3">
          {/* Tabs: Upcoming | Completed */}
          <div className="flex items-center gap-2">
            <button
              onClick={() => setActiveTab('upcoming')}
              className={`px-4 py-1.5 rounded-full font-mono uppercase tracking-wider text-xs transition-all cursor-pointer ${
                activeTab === 'upcoming'
                  ? 'bg-[#B600A8] text-white font-bold shadow-sm'
                  : 'bg-[#12151a] text-[#D7E2EA]/70 hover:text-white border border-[#D7E2EA]/15'
              }`}
            >
              Upcoming ({matches.filter((m) => m.status === 'upcoming').length})
            </button>

            <button
              onClick={() => setActiveTab('completed')}
              className={`px-4 py-1.5 rounded-full font-mono uppercase tracking-wider text-xs transition-all cursor-pointer ${
                activeTab === 'completed'
                  ? 'bg-[#B600A8] text-white font-bold shadow-sm'
                  : 'bg-[#12151a] text-[#D7E2EA]/70 hover:text-white border border-[#D7E2EA]/15'
              }`}
            >
              Completed ({matches.filter((m) => m.status === 'completed').length})
            </button>
          </div>

          {/* Filter Dropdowns */}
          <div className="flex flex-wrap items-center gap-2 text-xs">
            <select
              value={selectedCompFilter}
              onChange={(e) => setSelectedCompFilter(e.target.value)}
              className="px-3 py-1.5 rounded-full bg-[#12151a] border border-[#D7E2EA]/15 text-[#D7E2EA] focus:outline-none focus:border-[#B600A8] font-mono text-xs"
            >
              <option value="all">All Competitions</option>
              {competitions.map((c) => (
                <option key={c.id} value={c.id}>
                  {c.name}
                </option>
              ))}
            </select>

            {allRounds.length > 0 && (
              <select
                value={selectedRoundFilter}
                onChange={(e) => setSelectedRoundFilter(e.target.value)}
                className="px-3 py-1.5 rounded-full bg-[#12151a] border border-[#D7E2EA]/15 text-[#D7E2EA] focus:outline-none focus:border-[#B600A8] font-mono text-xs"
              >
                <option value="all">All Rounds</option>
                {allRounds.map((r) => (
                  <option key={r} value={r}>
                    {r}
                  </option>
                ))}
              </select>
            )}
          </div>
        </div>
      </FadeIn>

      {/* Tab: Upcoming Matches */}
      {activeTab === 'upcoming' && (
        <FadeIn delay={0.1} y={20} className="space-y-3">
          {filteredMatches.length > 0 ? (
            filteredMatches.map((match) => {
              const team1Name = getTeamName(match.team1Id, 'Falcons');
              const team2Name = getTeamName(match.team2Id, 'Mountain Strikers');
              const comp = competitions.find((c) => c.id === match.competitionId);

              return (
                <div
                  key={match.id}
                  className="p-5 rounded-2xl border border-[#D7E2EA]/15 bg-[#12151a] hover:bg-[#171a21] transition-all flex flex-col md:flex-row md:items-center justify-between gap-4"
                >
                  <div className="space-y-2">
                    <div className="flex items-center gap-2 text-xs font-mono text-[#D7E2EA]/60">
                      <span className="font-semibold text-white">
                        {match.roundName || 'Quarter Final 01'}
                      </span>
                      <span>•</span>
                      <span>{comp ? comp.name : 'Competition'}</span>
                    </div>

                    <div className="flex items-center gap-3 text-base sm:text-lg font-bold text-white">
                      <span>{team1Name}</span>
                      <span className="text-xs font-mono font-semibold px-2 py-0.5 rounded bg-[#171a21] border border-[#D7E2EA]/15 text-[#D7E2EA]/60">
                        VS
                      </span>
                      <span>{team2Name}</span>
                    </div>

                    <div className="flex items-center gap-3 text-xs font-mono text-[#D7E2EA]/60">
                      <span className="flex items-center gap-1.5">
                        <Calendar className="w-3.5 h-3.5 text-[#D7E2EA]/40" />
                        <span>Today • 4:30 PM</span>
                      </span>
                      <span>•</span>
                      <span>{match.venue || 'Main Arena'}</span>
                    </div>
                  </div>

                  <div className="shrink-0 flex items-center gap-3">
                    <button
                      onClick={() => onOpenResultModal(match)}
                      style={{
                        background: 'linear-gradient(123deg, #18011F 7%, #B600A8 37%, #7621B0 72%, #BE4C00 100%)',
                        boxShadow: '0px 4px 4px rgba(181, 1, 167, 0.25), 4px 4px 12px #7721B1 inset',
                        outline: '2px solid #ffffff',
                        outlineOffset: '-3px',
                      }}
                      className="w-full sm:w-auto inline-flex items-center justify-center gap-1.5 px-5 py-2 rounded-full text-white font-medium uppercase tracking-wider text-xs hover:opacity-90 transition-opacity cursor-pointer"
                    >
                      <span>Enter Result</span>
                      <ArrowRight className="w-3.5 h-3.5" />
                    </button>
                  </div>
                </div>
              );
            })
          ) : (
            <EmptyState
              title="No Upcoming Matches"
              description="All fixtures for this round or tournament have been completed."
            />
          )}
        </FadeIn>
      )}

      {/* Tab: Completed Matches */}
      {activeTab === 'completed' && (
        <FadeIn delay={0.1} y={20} className="space-y-3">
          {filteredMatches.length > 0 ? (
            filteredMatches.map((match) => {
              const team1Name = getTeamName(match.team1Id, 'Falcons');
              const team2Name = getTeamName(match.team2Id, 'Mountain Strikers');
              const comp = competitions.find((c) => c.id === match.competitionId);
              const scoreDisplay = `${match.team1Score ?? 0} — ${match.team2Score ?? 0}`;
              const isLatestCompleted = match.id === mostRecentCompletedMatchId && latestUndoable;

              return (
                <div
                  key={match.id}
                  className="p-5 rounded-2xl border border-[#D7E2EA]/15 bg-[#12151a] hover:bg-[#171a21] transition-all flex flex-col sm:flex-row sm:items-center justify-between gap-4"
                >
                  <div className="space-y-1.5">
                    <div className="flex items-center gap-3 text-sm sm:text-base font-semibold text-white">
                      <span>{team1Name}</span>
                      <span className="font-mono text-sm sm:text-base font-bold text-[#B600A8] px-3 py-0.5 rounded bg-[#18011F] border border-[#B600A8]/30 tabular-nums">
                        {scoreDisplay}
                      </span>
                      <span>{team2Name}</span>
                    </div>

                    <div className="flex items-center gap-2 text-xs font-mono text-[#D7E2EA]/60">
                      <span>{match.roundName || 'Quarter Final'}</span>
                      <span>•</span>
                      <span>{comp ? comp.name : 'Competition'}</span>
                      <span>•</span>
                      <span className="inline-flex items-center text-[10px] uppercase font-mono px-2 py-0.5 rounded-full bg-[#171a21] text-[#D7E2EA]/70 border border-[#D7E2EA]/15">
                        Completed
                      </span>
                    </div>
                  </div>

                  <div className="shrink-0 flex items-center gap-2">
                    {/* Subtle Undo button only on the most recently completed match */}
                    {isLatestCompleted && (
                      <button
                        onClick={() => setUndoTargetRecord(latestUndoable)}
                        className="inline-flex items-center gap-1.5 px-3 py-1.5 rounded-full text-xs font-mono uppercase tracking-wider text-[#D7E2EA] hover:bg-[#B600A8]/20 border border-[#B600A8]/40 transition-colors cursor-pointer"
                        title="Revert this match result"
                      >
                        <RotateCcw className="w-3 h-3 text-[#B600A8]" />
                        <span>Undo last result</span>
                      </button>
                    )}

                    <button
                      onClick={() => onOpenResultModal(match)}
                      className="px-4 py-1.5 rounded-full text-xs font-medium uppercase tracking-wider border-2 border-[#D7E2EA] text-[#D7E2EA] hover:bg-[#D7E2EA]/10 transition-colors cursor-pointer"
                    >
                      View Result
                    </button>
                  </div>
                </div>
              );
            })
          ) : (
            <EmptyState
              title="No Completed Matches"
              description="Recorded match results will appear here with final scores."
            />
          )}
        </FadeIn>
      )}

      {/* Undo Confirmation Modal */}
      <Modal
        isOpen={!!undoTargetRecord}
        onClose={() => setUndoTargetRecord(null)}
        title="Undo this result?"
        maxWidth="md"
      >
        <div className="space-y-4 text-xs">
          <p className="text-[#D7E2EA]/80 leading-relaxed">
            The leaderboard and qualification status will be restored to the previous state.
          </p>

          {undoTargetRecord && (
            <div className="p-3.5 rounded-xl bg-[#0C0C0C] border border-[#D7E2EA]/15 font-mono text-[11px] text-[#D7E2EA] space-y-1">
              <div className="text-[#D7E2EA]/50 uppercase tracking-wider text-[10px]">Action:</div>
              <div className="font-semibold text-white">
                {undoTargetRecord.description || 'Recent Match Result'}
              </div>
            </div>
          )}

          <div className="flex items-center justify-end gap-2.5 pt-3 border-t border-[#D7E2EA]/15">
            <button
              type="button"
              onClick={() => setUndoTargetRecord(null)}
              className="px-4 py-2 rounded-full border-2 border-[#D7E2EA] text-[#D7E2EA] hover:bg-[#D7E2EA]/10 transition-colors font-mono text-xs uppercase cursor-pointer"
            >
              Cancel
            </button>
            <button
              type="button"
              onClick={handleConfirmUndo}
              className="px-4 py-2 rounded-full bg-rose-500/20 text-rose-300 border border-rose-500/40 hover:bg-rose-500/30 transition-colors font-mono text-xs uppercase font-semibold cursor-pointer"
            >
              Undo Result
            </button>
          </div>
        </div>
      </Modal>
    </div>
  );
};
