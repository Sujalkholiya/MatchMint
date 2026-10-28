import React, { useState } from 'react';
import { Competition, Team, Match, HistoryRecord } from '../types';
import { StatusBadge } from '../components/common/StatusBadge';
import { FormatBadge } from '../components/common/FormatBadge';
import { FixtureBracket } from '../components/competition/FixtureBracket';
import { EmptyState } from '../components/common/EmptyState';
import { FadeIn } from '../components/landing/FadeIn';
import {
  ArrowLeft,
  Calendar,
  RotateCcw,
  Users,
} from 'lucide-react';

interface CompetitionDetailsProps {
  competition: Competition;
  teams: Team[];
  matches: Match[];
  history?: HistoryRecord[];
  onBack: () => void;
  onOpenResultModal: (match: Match) => void;
  onGenerateFixtures: (comp: Competition) => void;
  onUndoResult?: (historyId: string) => void;
}

export const CompetitionDetails: React.FC<CompetitionDetailsProps> = ({
  competition,
  teams,
  matches,
  history = [],
  onBack,
  onOpenResultModal,
  onGenerateFixtures,
  onUndoResult,
}) => {
  const isRoundRobin = competition.format === 'round_robin';
  const [activeTab, setActiveTab] = useState<'bracket' | 'standings' | 'teams' | 'activity'>('bracket');

  const compTeams = teams
    .filter((t) => t.competitionId === competition.id)
    .sort((a, b) => b.points - a.points || b.scoreDiff - a.scoreDiff);

  const compMatches = matches.filter((m) => m.competitionId === competition.id);
  const compHistory = history
    .filter((h) => h.competitionId === competition.id)
    .slice(0, 20);

  const getTeamName = (teamId: string | null) => {
    if (!teamId) return 'TBD';
    const found = teams.find((t) => t.id === teamId);
    return found ? found.name : 'TBD';
  };

  return (
    <div className="p-6 max-w-5xl mx-auto space-y-6">
      {/* Top Header */}
      <FadeIn delay={0} y={20}>
        <div className="space-y-3 pb-4 border-b border-[#D7E2EA]/15">
          <button
            onClick={onBack}
            className="inline-flex items-center gap-1.5 text-xs text-[#D7E2EA]/60 hover:text-white transition-colors cursor-pointer"
          >
            <ArrowLeft className="w-3.5 h-3.5" />
            <span>Back to Competitions</span>
          </button>

          <div className="flex flex-col sm:flex-row sm:items-center justify-between gap-4">
            <div className="space-y-1.5">
              <div className="flex items-center gap-2">
                <StatusBadge status={competition.status} size="sm" />
                <FormatBadge format={competition.format} />
              </div>
              <h1 className="hero-heading text-2xl sm:text-4xl font-black uppercase tracking-tight">
                {competition.name}
              </h1>
              <p className="text-xs text-[#D7E2EA]/60">
                {competition.description}
              </p>
            </div>

            {compMatches.length === 0 && (
              <button
                onClick={() => onGenerateFixtures(competition)}
                style={{
                  background: 'linear-gradient(123deg, #18011F 7%, #B600A8 37%, #7621B0 72%, #BE4C00 100%)',
                  boxShadow: '0px 4px 4px rgba(181, 1, 167, 0.25), 4px 4px 12px #7721B1 inset',
                  outline: '2px solid #ffffff',
                  outlineOffset: '-3px',
                }}
                className="px-5 py-2 text-xs font-medium uppercase tracking-wider rounded-full text-white hover:opacity-90 transition-opacity shrink-0 cursor-pointer"
              >
                Generate Fixtures
              </button>
            )}
          </div>

          {/* Tab Navigation: Bracket/Fixtures | Standings | Teams | Activity */}
          <nav className="flex items-center gap-2 pt-3 border-t border-[#D7E2EA]/10 text-xs">
            <button
              onClick={() => setActiveTab('bracket')}
              className={`px-4 py-1.5 rounded-full font-mono uppercase tracking-wider text-xs transition-all cursor-pointer ${
                activeTab === 'bracket'
                  ? 'bg-[#B600A8] text-white font-bold shadow-sm'
                  : 'bg-[#12151a] text-[#D7E2EA]/70 hover:text-white border border-[#D7E2EA]/15'
              }`}
            >
              {isRoundRobin ? 'Fixtures' : 'Bracket'}
            </button>

            <button
              onClick={() => setActiveTab('standings')}
              className={`px-4 py-1.5 rounded-full font-mono uppercase tracking-wider text-xs transition-all cursor-pointer ${
                activeTab === 'standings'
                  ? 'bg-[#B600A8] text-white font-bold shadow-sm'
                  : 'bg-[#12151a] text-[#D7E2EA]/70 hover:text-white border border-[#D7E2EA]/15'
              }`}
            >
              Standings
            </button>

            <button
              onClick={() => setActiveTab('teams')}
              className={`px-4 py-1.5 rounded-full font-mono uppercase tracking-wider text-xs transition-all cursor-pointer ${
                activeTab === 'teams'
                  ? 'bg-[#B600A8] text-white font-bold shadow-sm'
                  : 'bg-[#12151a] text-[#D7E2EA]/70 hover:text-white border border-[#D7E2EA]/15'
              }`}
            >
              Teams ({compTeams.length})
            </button>

            <button
              onClick={() => setActiveTab('activity')}
              className={`px-4 py-1.5 rounded-full font-mono uppercase tracking-wider text-xs transition-all cursor-pointer ${
                activeTab === 'activity'
                  ? 'bg-[#B600A8] text-white font-bold shadow-sm'
                  : 'bg-[#12151a] text-[#D7E2EA]/70 hover:text-white border border-[#D7E2EA]/15'
              }`}
            >
              Activity ({compHistory.length})
            </button>
          </nav>
        </div>
      </FadeIn>

      {/* Tab 1: Bracket or Round-Robin Fixtures */}
      {activeTab === 'bracket' && (
        <FadeIn delay={0.05} y={20} className="space-y-4">
          {compMatches.length === 0 ? (
            <EmptyState
              icon={Calendar}
              title="Fixtures not generated"
              description="Click the button above to generate fixtures for this tournament."
              action={{
                label: 'Generate Fixtures',
                onClick: () => onGenerateFixtures(competition),
              }}
            />
          ) : isRoundRobin ? (
            <div className="space-y-3">
              {compMatches.map((match) => (
                <div
                  key={match.id}
                  className="p-4 rounded-2xl border border-[#D7E2EA]/15 bg-[#12151a] flex items-center justify-between gap-4"
                >
                  <div className="space-y-1">
                    <span className="font-mono text-[11px] text-[#D7E2EA]/60">
                      {match.roundName} • {match.venue}
                    </span>
                    <div className="text-sm font-semibold text-white flex items-center gap-2">
                      <span>{getTeamName(match.team1Id)}</span>
                      {match.status === 'completed' ? (
                        <span className="font-mono text-[#B600A8] font-bold px-2.5 py-0.5 rounded bg-[#18011F] border border-[#B600A8]/30 tabular-nums">
                          {match.team1Score} — {match.team2Score}
                        </span>
                      ) : (
                        <span className="text-xs text-[#D7E2EA]/40 font-mono">vs</span>
                      )}
                      <span>{getTeamName(match.team2Id)}</span>
                    </div>
                  </div>

                  <button
                    onClick={() => onOpenResultModal(match)}
                    className="px-3.5 py-1.5 rounded-full text-xs font-medium uppercase tracking-wider border-2 border-[#D7E2EA] text-[#D7E2EA] hover:bg-[#D7E2EA]/10 transition-colors cursor-pointer"
                  >
                    {match.status === 'completed' ? 'Edit Result' : 'Enter Result'}
                  </button>
                </div>
              ))}
            </div>
          ) : (
            <div className="p-4 rounded-2xl border border-[#D7E2EA]/15 bg-[#12151a] overflow-x-auto">
              <FixtureBracket
                matches={compMatches}
                teams={compTeams}
                onOpenResultModal={onOpenResultModal}
              />
            </div>
          )}
        </FadeIn>
      )}

      {/* Tab 2: Standings Table (1st place subtle magenta left border, no gradients) */}
      {activeTab === 'standings' && (
        <FadeIn delay={0.05} y={20} className="overflow-x-auto rounded-2xl border border-[#D7E2EA]/15 bg-[#12151a]">
          <table className="w-full text-left border-collapse text-xs">
            <thead>
              <tr className="border-b border-[#D7E2EA]/15 bg-[#0C0C0C]/80 font-mono text-[11px] text-[#D7E2EA]/60 uppercase tracking-wider">
                <th className="py-3 px-4 w-16 text-center">Rank</th>
                <th className="py-3 px-4">Team</th>
                <th className="py-3 px-4 text-center">P</th>
                <th className="py-3 px-4 text-center">W</th>
                <th className="py-3 px-4 text-center">L</th>
                <th className="py-3 px-4 text-right font-bold text-white">Pts</th>
              </tr>
            </thead>
            <tbody className="divide-y divide-[#D7E2EA]/10">
              {compTeams.map((team) => {
                const isFirst = team.rank === 1;
                const played = team.wins + team.draws + team.losses;

                return (
                  <tr
                    key={team.id}
                    className={`hover:bg-[#171a21] transition-colors ${
                      isFirst ? 'border-l-4 border-l-[#B600A8]' : ''
                    }`}
                  >
                    <td className="py-3 px-4 text-center font-mono font-bold text-[#D7E2EA]/80">
                      {team.rank}
                    </td>
                    <td className="py-3 px-4 font-semibold text-white">
                      {team.name}
                    </td>
                    <td className="py-3 px-4 text-center font-mono tabular-nums text-[#D7E2EA]/60">
                      {played}
                    </td>
                    <td className="py-3 px-4 text-center font-mono tabular-nums text-white">
                      {team.wins}
                    </td>
                    <td className="py-3 px-4 text-center font-mono tabular-nums text-[#D7E2EA]/60">
                      {team.losses}
                    </td>
                    <td className="py-3 px-4 text-right font-mono tabular-nums font-bold text-[#B600A8] text-sm">
                      {team.points}
                    </td>
                  </tr>
                );
              })}
            </tbody>
          </table>
        </FadeIn>
      )}

      {/* Tab 3: Teams Table (Team, Captain, Members) */}
      {activeTab === 'teams' && (
        <FadeIn delay={0.05} y={20} className="overflow-x-auto rounded-2xl border border-[#D7E2EA]/15 bg-[#12151a]">
          <table className="w-full text-left border-collapse text-xs">
            <thead>
              <tr className="border-b border-[#D7E2EA]/15 bg-[#0C0C0C]/80 font-mono text-[11px] text-[#D7E2EA]/60 uppercase tracking-wider">
                <th className="py-3 px-4">Team</th>
                <th className="py-3 px-4">Captain</th>
                <th className="py-3 px-4 text-center">Members</th>
              </tr>
            </thead>
            <tbody className="divide-y divide-[#D7E2EA]/10">
              {compTeams.map((team) => (
                <tr key={team.id} className="hover:bg-[#171a21] transition-colors">
                  <td className="py-3 px-4 font-semibold text-white">
                    {team.name}
                  </td>
                  <td className="py-3 px-4 text-[#D7E2EA]">
                    {team.captain}
                  </td>
                  <td className="py-3 px-4 text-center font-mono text-[#D7E2EA]/60 tabular-nums">
                    {team.rosterCount}
                  </td>
                </tr>
              ))}
            </tbody>
          </table>
        </FadeIn>
      )}

      {/* Tab 4: Activity (Small Recent Activity list, max 20 events with Undo on latest result) */}
      {activeTab === 'activity' && (
        <FadeIn delay={0.05} y={20} className="space-y-3">
          {compHistory.length > 0 ? (
            <div className="rounded-2xl border border-[#D7E2EA]/15 bg-[#12151a] divide-y divide-[#D7E2EA]/10 overflow-hidden">
              {compHistory.map((rec) => (
                <div
                  key={rec.id}
                  className="p-3.5 flex items-center justify-between text-xs gap-3 hover:bg-[#171a21] transition-colors"
                >
                  <div className="space-y-0.5">
                    <div className="flex items-center gap-2 font-mono text-[11px] text-[#D7E2EA]/50">
                      <span>{rec.timestamp}</span>
                      <span>•</span>
                      <span className="text-[#B600A8] font-bold">{rec.actionType.replace('_', ' ')}</span>
                    </div>
                    <div className="text-white font-medium">
                      {rec.description}
                    </div>
                  </div>

                  {rec.canUndo && onUndoResult && (
                    <button
                      onClick={() => onUndoResult(rec.id)}
                      className="px-3 py-1 text-xs font-mono uppercase tracking-wider rounded-full bg-[#18011F] hover:bg-[#B600A8]/20 text-[#D7E2EA] border border-[#B600A8]/40 transition-colors flex items-center gap-1.5 cursor-pointer"
                    >
                      <RotateCcw className="w-3 h-3 text-[#B600A8]" />
                      <span>Undo</span>
                    </button>
                  )}
                </div>
              ))}
            </div>
          ) : (
            <div className="p-8 rounded-2xl border border-dashed border-[#D7E2EA]/15 bg-[#12151a]/40 text-center text-xs text-[#D7E2EA]/60">
              No recorded activity yet.
            </div>
          )}
        </FadeIn>
      )}
    </div>
  );
};
