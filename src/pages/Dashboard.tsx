import React from 'react';
import { Competition, Team, Match } from '../types';
import { StatusBadge } from '../components/common/StatusBadge';
import { FormatBadge } from '../components/common/FormatBadge';
import { ArrowRight, Clock, MapPin, Trophy, Users, Calendar } from 'lucide-react';
import { FadeIn } from '../components/landing/FadeIn';

interface DashboardProps {
  competitions: Competition[];
  teams: Team[];
  matches: Match[];
  onSelectCompetition: (comp: Competition) => void;
  onOpenResultModal: (match: Match) => void;
}

export const Dashboard: React.FC<DashboardProps> = ({
  competitions,
  teams,
  matches,
  onSelectCompetition,
  onOpenResultModal,
}) => {
  const featuredComp =
    competitions.find((c) => c.id === 'comp-gehu-2026') ||
    competitions.find((c) => c.status === 'live') ||
    competitions[0];

  const featuredTeams = featuredComp
    ? teams.filter((t) => t.competitionId === featuredComp.id)
    : [];

  const upcomingMatches = matches
    .filter((m) => m.status === 'upcoming' && m.team1Id && m.team2Id)
    .slice(0, 4);

  const getTeamName = (teamId: string | null) => {
    if (!teamId) return 'TBD';
    const found = teams.find((t) => t.id === teamId);
    return found ? found.name : 'TBD';
  };

  return (
    <div className="p-6 max-w-5xl mx-auto space-y-6">
      {/* Page Header */}
      <FadeIn delay={0} y={20}>
        <div className="pb-2">
          <h1 className="hero-heading text-3xl md:text-5xl font-black uppercase tracking-tight">
            Dashboard
          </h1>
          <p className="text-xs text-[#D7E2EA]/60 mt-1">
            Realtime tournament summary and match operations.
          </p>
        </div>
      </FadeIn>

      {/* 1. Featured Competition Card */}
      {featuredComp && (
        <FadeIn delay={0.05} y={20}>
          <div className="p-6 rounded-2xl bg-[#12151a] border border-[#D7E2EA]/15 space-y-4 shadow-sm">
            <div className="flex flex-col sm:flex-row sm:items-center justify-between gap-4">
              <div className="space-y-1.5">
                <div className="flex items-center gap-2">
                  <StatusBadge status={featuredComp.status} size="sm" />
                  <FormatBadge format={featuredComp.format} />
                </div>
                <h2 className="text-xl sm:text-2xl font-bold tracking-tight text-white">
                  {featuredComp.name}
                </h2>
                <p className="text-xs text-[#D7E2EA]/60">
                  {featuredComp.description}
                </p>
              </div>

              <div className="flex items-center gap-2 shrink-0">
                <button
                  onClick={() => onSelectCompetition(featuredComp)}
                  style={{
                    background: 'linear-gradient(123deg, #18011F 7%, #B600A8 37%, #7621B0 72%, #BE4C00 100%)',
                    boxShadow: '0px 4px 4px rgba(181, 1, 167, 0.25), 4px 4px 12px #7721B1 inset',
                    outline: '2px solid #ffffff',
                    outlineOffset: '-3px',
                  }}
                  className="px-5 py-2 text-xs font-medium uppercase tracking-wider rounded-full text-white hover:opacity-90 transition-opacity inline-flex items-center gap-1.5 cursor-pointer"
                >
                  <span>Open Tournament</span>
                  <ArrowRight className="w-3.5 h-3.5" />
                </button>
              </div>
            </div>

            <div className="grid grid-cols-2 sm:grid-cols-4 gap-3 pt-3 border-t border-[#D7E2EA]/10">
              <div>
                <span className="text-[11px] font-mono text-[#D7E2EA]/50 block">Round</span>
                <span className="text-xs font-semibold text-white mt-0.5 block">
                  Round {featuredComp.currentRound} of {featuredComp.totalRounds}
                </span>
              </div>
              <div>
                <span className="text-[11px] font-mono text-[#D7E2EA]/50 block">Teams</span>
                <span className="text-xs font-semibold text-white mt-0.5 block font-mono tabular-nums">
                  {featuredTeams.length} / {featuredComp.totalTeams}
                </span>
              </div>
              <div>
                <span className="text-[11px] font-mono text-[#D7E2EA]/50 block">Discipline</span>
                <span className="text-xs font-semibold text-white mt-0.5 block capitalize">
                  {featuredComp.category}
                </span>
              </div>
              <div>
                <span className="text-[11px] font-mono text-[#D7E2EA]/50 block">Format</span>
                <span className="text-xs font-semibold text-white mt-0.5 block">
                  {featuredComp.format === 'round_robin' ? 'Round Robin' : 'Knockout'}
                </span>
              </div>
            </div>
          </div>
        </FadeIn>
      )}

      {/* 2. 3-Stat Metric Row */}
      <FadeIn delay={0.1} y={20}>
        <div className="grid grid-cols-1 sm:grid-cols-3 gap-4">
          <div className="p-4 rounded-2xl bg-[#12151a] border border-[#D7E2EA]/15 flex items-center justify-between">
            <div>
              <span className="text-xs font-mono uppercase text-[#D7E2EA]/60 tracking-wider">
                Active Tournaments
              </span>
              <div className="text-2xl font-bold font-mono text-white mt-1 tabular-nums">
                {competitions.length}
              </div>
            </div>
            <div className="w-10 h-10 rounded-xl bg-gradient-to-br from-[#18011F] to-[#7621B0]/30 border border-[#B600A8]/30 flex items-center justify-center text-[#B600A8]">
              <Trophy className="w-5 h-5" />
            </div>
          </div>

          <div className="p-4 rounded-2xl bg-[#12151a] border border-[#D7E2EA]/15 flex items-center justify-between">
            <div>
              <span className="text-xs font-mono uppercase text-[#D7E2EA]/60 tracking-wider">
                Registered Squads
              </span>
              <div className="text-2xl font-bold font-mono text-white mt-1 tabular-nums">
                {teams.length}
              </div>
            </div>
            <div className="w-10 h-10 rounded-xl bg-gradient-to-br from-[#18011F] to-[#7621B0]/30 border border-[#B600A8]/30 flex items-center justify-center text-[#7621B0]">
              <Users className="w-5 h-5" />
            </div>
          </div>

          <div className="p-4 rounded-2xl bg-[#12151a] border border-[#D7E2EA]/15 flex items-center justify-between">
            <div>
              <span className="text-xs font-mono uppercase text-[#D7E2EA]/60 tracking-wider">
                Upcoming Matches
              </span>
              <div className="text-2xl font-bold font-mono text-white mt-1 tabular-nums">
                {matches.filter((m) => m.status === 'upcoming').length}
              </div>
            </div>
            <div className="w-10 h-10 rounded-xl bg-gradient-to-br from-[#18011F] to-[#7621B0]/30 border border-[#B600A8]/30 flex items-center justify-center text-[#BE4C00]">
              <Calendar className="w-5 h-5" />
            </div>
          </div>
        </div>
      </FadeIn>

      {/* 3. Today's Matches */}
      <FadeIn delay={0.15} y={20}>
        <div className="space-y-3">
          <div className="flex items-center justify-between">
            <h3 className="text-sm font-bold uppercase tracking-wider text-white">
              Today's Matches
            </h3>
            <span className="text-xs font-mono text-[#D7E2EA]/50">
              {upcomingMatches.length} pending
            </span>
          </div>

          {upcomingMatches.length === 0 ? (
            <div className="p-8 rounded-2xl border border-dashed border-[#D7E2EA]/15 bg-[#12151a]/40 text-center">
              <p className="text-xs text-[#D7E2EA]/60">No pending matches scheduled for today.</p>
            </div>
          ) : (
            <div className="space-y-2.5">
              {upcomingMatches.map((m) => (
                <div
                  key={m.id}
                  className="flex flex-col sm:flex-row sm:items-center justify-between p-4 rounded-2xl border border-[#D7E2EA]/15 bg-[#12151a] hover:bg-[#171a21] transition-all gap-3"
                >
                  <div className="space-y-1">
                    <div className="flex items-center gap-2 text-xs font-mono text-[#D7E2EA]/60">
                      <span>{m.roundName}</span>
                      <span>•</span>
                      <span className="flex items-center gap-1">
                        <Clock className="w-3 h-3 text-[#D7E2EA]/40" />
                        <span>Today • 4:30 PM</span>
                      </span>
                    </div>
                    <div className="text-sm sm:text-base font-bold text-white flex items-center gap-2">
                      <span>{getTeamName(m.team1Id)}</span>
                      <span className="text-xs font-mono text-[#D7E2EA]/50">VS</span>
                      <span>{getTeamName(m.team2Id)}</span>
                    </div>
                  </div>

                  <button
                    onClick={() => onOpenResultModal(m)}
                    className="self-start sm:self-auto px-4 py-1.5 rounded-full text-xs font-medium uppercase tracking-wider border-2 border-[#D7E2EA] text-[#D7E2EA] hover:bg-[#D7E2EA]/10 transition-colors cursor-pointer"
                  >
                    Enter Result
                  </button>
                </div>
              ))}
            </div>
          )}
        </div>
      </FadeIn>
    </div>
  );
};
