import React from 'react';
import { Match, Team } from '../../types';
import { Trophy } from 'lucide-react';

interface FixtureBracketProps {
  matches: Match[];
  teams: Team[];
  onOpenResultModal: (match: Match) => void;
}

export const FixtureBracket: React.FC<FixtureBracketProps> = ({
  matches,
  teams,
  onOpenResultModal,
}) => {
  const rounds = Array.from(new Set(matches.map((m) => m.round))).sort((a, b) => a - b);

  if (matches.length === 0) {
    return (
      <div className="p-8 text-center text-[#D7E2EA]/50 font-mono text-xs">
        No tournament bracket fixtures generated yet.
      </div>
    );
  }

  const getTeam = (teamId: string | null) => teams.find((t) => t.id === teamId);

  // Find the next playable match
  const nextPlayableMatch = matches.find(
    (m) => m.status === 'upcoming' && m.team1Id && m.team2Id
  );

  return (
    <div className="w-full overflow-x-auto pb-4 pt-2">
      <div className="flex items-stretch gap-8 sm:gap-10 min-w-max px-2">
        {rounds.map((roundNum) => {
          const roundMatches = matches
            .filter((m) => m.round === roundNum)
            .sort((a, b) => a.matchNumber - b.matchNumber);

          const roundTitle =
            roundMatches[0]?.roundName.split(' ')[0] || `Round ${roundNum}`;

          return (
            <div key={roundNum} className="flex flex-col w-64">
              {/* Round Header */}
              <div className="mb-4 pb-2 border-b border-[#D7E2EA]/15 flex items-center justify-between">
                <div>
                  <h4 className="text-xs font-mono font-semibold uppercase tracking-wider text-white">
                    {roundTitle}
                  </h4>
                  <span className="text-[10px] text-[#D7E2EA]/50 font-mono">
                    {roundMatches.length} Fixture{roundMatches.length > 1 ? 's' : ''}
                  </span>
                </div>
                {roundNum === rounds[rounds.length - 1] && (
                  <span className="flex items-center gap-1 text-[10px] font-mono text-[#B600A8] bg-[#B600A8]/10 px-2 py-0.5 rounded-full border border-[#B600A8]/30">
                    <Trophy className="w-3 h-3 text-[#B600A8]" /> Final
                  </span>
                )}
              </div>

              {/* Bracket Nodes */}
              <div className="flex flex-col justify-around flex-1 space-y-6">
                {roundMatches.map((match) => {
                  const t1 = getTeam(match.team1Id);
                  const t2 = getTeam(match.team2Id);
                  const isPlayable = match.team1Id && match.team2Id;
                  const isNextPlayable = nextPlayableMatch?.id === match.id;
                  const t1Won = match.status === 'completed' && match.winnerId === match.team1Id;
                  const t2Won = match.status === 'completed' && match.winnerId === match.team2Id;

                  return (
                    <div key={match.id} className="relative">
                      <div
                        className={`rounded-2xl border bg-[#12151a] p-3 transition-colors ${
                          isNextPlayable
                            ? 'border-[#B600A8] ring-1 ring-[#B600A8]/40 shadow-[0_0_15px_rgba(182,0,168,0.2)]'
                            : 'border-[#D7E2EA]/15'
                        } ${!isPlayable ? 'opacity-50' : ''}`}
                      >
                        {/* Match header */}
                        <div className="flex items-center justify-between text-[10px] font-mono text-[#D7E2EA]/50 mb-2 pb-1 border-b border-[#D7E2EA]/10">
                          <span>#{match.matchNumber} {match.roundName}</span>
                          <span className="truncate">{match.scheduledTime.split(' ')[1] || ''}</span>
                        </div>

                        {/* Team 1 */}
                        <div
                          className={`flex items-center justify-between px-2.5 py-1 rounded-lg text-xs ${
                            t1Won
                              ? 'bg-[#B600A8]/20 text-white font-bold'
                              : match.status === 'completed'
                              ? 'text-[#D7E2EA]/40'
                              : 'text-white'
                          }`}
                        >
                          <span className="truncate">
                            {t1 ? `${t1.name}` : 'TBD'}
                          </span>
                          <span className="font-mono tabular-nums font-bold ml-2">
                            {match.team1Score !== null ? match.team1Score : '—'}
                          </span>
                        </div>

                        {/* Team 2 */}
                        <div
                          className={`flex items-center justify-between px-2.5 py-1 rounded-lg text-xs mt-1 ${
                            t2Won
                              ? 'bg-[#B600A8]/20 text-white font-bold'
                              : match.status === 'completed'
                              ? 'text-[#D7E2EA]/40'
                              : 'text-white'
                          }`}
                        >
                          <span className="truncate">
                            {t2 ? `${t2.name}` : 'TBD'}
                          </span>
                          <span className="font-mono tabular-nums font-bold ml-2">
                            {match.team2Score !== null ? match.team2Score : '—'}
                          </span>
                        </div>

                        {/* Action on next playable match or completed matches */}
                        {isPlayable && (
                          <div className="mt-2.5 pt-2 border-t border-[#D7E2EA]/10 flex items-center justify-end">
                            <button
                              type="button"
                              onClick={() => onOpenResultModal(match)}
                              style={
                                isNextPlayable
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
                              className={`text-[10px] font-mono uppercase tracking-wider transition-colors cursor-pointer ${
                                isNextPlayable
                                  ? 'px-3 py-1 rounded-full text-white font-bold hover:opacity-90'
                                  : 'text-[#D7E2EA]/60 hover:text-white'
                              }`}
                            >
                              {match.status === 'completed' ? 'Edit Result' : 'Enter Result'}
                            </button>
                          </div>
                        )}
                      </div>

                      {/* Connecting Line to next match */}
                      {match.nextMatchId && (
                        <div className="hidden sm:block absolute top-1/2 -right-8 w-8 h-px bg-[#D7E2EA]/20 pointer-events-none" />
                      )}
                    </div>
                  );
                })}
              </div>
            </div>
          );
        })}
      </div>
    </div>
  );
};
