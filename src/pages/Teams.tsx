import React, { useState } from 'react';
import { Team, Competition } from '../types';
import { EmptyState } from '../components/common/EmptyState';
import { Shield, Plus } from 'lucide-react';
import { FadeIn } from '../components/landing/FadeIn';

interface TeamsProps {
  teams: Team[];
  competitions: Competition[];
  onOpenRegisterModal: () => void;
  onSelectCompetition?: (comp: Competition) => void;
}

export const Teams: React.FC<TeamsProps> = ({
  teams,
  competitions,
  onOpenRegisterModal,
  onSelectCompetition,
}) => {
  const [selectedCompFilter, setSelectedCompFilter] = useState<string>('all');

  const filteredTeams = teams.filter((t) => {
    if (selectedCompFilter !== 'all' && t.competitionId !== selectedCompFilter) return false;
    return true;
  });

  return (
    <div className="p-6 max-w-6xl mx-auto space-y-6">
      {/* Header */}
      <FadeIn delay={0} y={20}>
        <div className="flex flex-col sm:flex-row sm:items-center justify-between gap-4 pb-2">
          <div>
            <h1 className="hero-heading text-3xl md:text-5xl font-black uppercase tracking-tight">
              Teams
            </h1>
            <p className="text-xs text-[#D7E2EA]/60 mt-1">
              Registered squads across all active competitions.
            </p>
          </div>

          <div className="flex items-center gap-3 self-start sm:self-auto">
            {/* Competition Filter */}
            <select
              value={selectedCompFilter}
              onChange={(e) => setSelectedCompFilter(e.target.value)}
              className="px-3.5 py-2 bg-[#12151a] border border-[#D7E2EA]/15 rounded-full text-xs text-[#D7E2EA] focus:outline-none focus:border-[#B600A8] font-mono cursor-pointer"
            >
              <option value="all">All Competitions ({teams.length})</option>
              {competitions.map((c) => (
                <option key={c.id} value={c.id}>
                  {c.name}
                </option>
              ))}
            </select>

            {/* Add Team Button */}
            <button
              onClick={onOpenRegisterModal}
              style={{
                background: 'linear-gradient(123deg, #18011F 7%, #B600A8 37%, #7621B0 72%, #BE4C00 100%)',
                boxShadow: '0px 4px 4px rgba(181, 1, 167, 0.25), 4px 4px 12px #7721B1 inset',
                outline: '2px solid #ffffff',
                outlineOffset: '-3px',
              }}
              className="inline-flex items-center gap-1.5 px-4 py-2 text-xs font-medium uppercase tracking-wider rounded-full text-white hover:opacity-90 transition-opacity cursor-pointer shadow-sm shrink-0"
            >
              <Plus className="w-3.5 h-3.5" />
              <span>Add Team</span>
            </button>
          </div>
        </div>
      </FadeIn>

      {/* Teams Table */}
      <FadeIn delay={0.05} y={20}>
        {filteredTeams.length === 0 ? (
          <EmptyState
            title="No Teams Found"
            description={
              selectedCompFilter !== 'all'
                ? 'No teams registered in this competition yet.'
                : 'Register your first squad to get started.'
            }
            actionLabel="Add Team"
            onAction={onOpenRegisterModal}
          />
        ) : (
          <div className="bg-[#12151a] border border-[#D7E2EA]/15 rounded-2xl overflow-hidden shadow-sm">
            <div className="overflow-x-auto">
              <table className="w-full text-left border-collapse text-xs">
                <thead>
                  <tr className="border-b border-[#D7E2EA]/15 bg-[#0C0C0C]/80 text-[#D7E2EA]/60 font-mono uppercase tracking-wider text-[11px]">
                    <th className="py-3.5 px-4">Team</th>
                    <th className="py-3.5 px-4">Captain</th>
                    <th className="py-3.5 px-4 text-center">Members</th>
                    <th className="py-3.5 px-4">Competition</th>
                  </tr>
                </thead>
                <tbody className="divide-y divide-[#D7E2EA]/10">
                  {filteredTeams.map((team) => {
                    const comp = competitions.find((c) => c.id === team.competitionId);
                    return (
                      <tr
                        key={team.id}
                        className="hover:bg-[#171a21] transition-colors"
                      >
                        {/* Team Name & Tag */}
                        <td className="py-3.5 px-4">
                          <div className="flex items-center gap-3">
                            <div className="w-8 h-8 rounded-xl bg-gradient-to-br from-[#18011F] to-[#7621B0]/30 border border-[#B600A8]/30 flex items-center justify-center shrink-0">
                              <Shield className="w-4 h-4 text-[#B600A8]" />
                            </div>
                            <div>
                              <div className="font-semibold text-white text-sm">{team.name}</div>
                              <div className="font-mono text-[10px] text-[#D7E2EA]/50 tracking-wider">
                                [{team.shortTag}]
                              </div>
                            </div>
                          </div>
                        </td>

                        {/* Captain */}
                        <td className="py-3.5 px-4 text-[#D7E2EA] font-medium">
                          {team.captain}
                        </td>

                        {/* Members */}
                        <td className="py-3.5 px-4 text-center font-mono text-[#D7E2EA]/70 tabular-nums">
                          {team.rosterCount || 5}
                        </td>

                        {/* Competition */}
                        <td className="py-3.5 px-4">
                          {comp ? (
                            <button
                              type="button"
                              onClick={() => onSelectCompetition && onSelectCompetition(comp)}
                              className="text-[#B600A8] hover:text-white font-medium hover:underline text-left transition-colors cursor-pointer"
                            >
                              {comp.name}
                            </button>
                          ) : (
                            <span className="text-[#D7E2EA]/40 font-mono">—</span>
                          )}
                        </td>
                      </tr>
                    );
                  })}
                </tbody>
              </table>
            </div>
            <div className="px-4 py-3 bg-[#0C0C0C]/40 border-t border-[#D7E2EA]/15 text-[11px] font-mono text-[#D7E2EA]/50 flex items-center justify-between">
              <span>Total: {filteredTeams.length} {filteredTeams.length === 1 ? 'team' : 'teams'}</span>
              <span>Squad Records</span>
            </div>
          </div>
        )}
      </FadeIn>
    </div>
  );
};
