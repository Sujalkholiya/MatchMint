import React, { useState } from 'react';
import { CompetitionCategory, TournamentFormat, Competition } from '../types';
import { StorageService } from '../services/storageService';
import { Trophy, Gamepad2, ArrowRight, ArrowLeft, Check, Calendar, Clock, Sparkles } from 'lucide-react';
import { FadeIn } from '../components/landing/FadeIn';

interface CreateCompetitionProps {
  onCreated: (newComp: Competition, autoGenerateFixtures?: boolean) => void;
  onCancel: () => void;
}

type Step = 1 | 2 | 3;

export const CreateCompetition: React.FC<CreateCompetitionProps> = ({
  onCreated,
  onCancel,
}) => {
  const [step, setStep] = useState<Step>(1);
  const [name, setName] = useState('');
  const [category, setCategory] = useState<CompetitionCategory>('sports');
  const [format, setFormat] = useState<TournamentFormat>('single_elimination');
  const [teamCount, setTeamCount] = useState<number>(8);
  const [startDate, setStartDate] = useState('2026-10-30');
  const [startTime, setStartTime] = useState('10:00 AM');
  const [durationMinutes, setDurationMinutes] = useState<number>(60);
  const [generateNow, setGenerateNow] = useState<boolean>(true);
  const [error, setError] = useState<string | null>(null);

  const handleNext = () => {
    setError(null);
    if (step === 1) {
      if (!name.trim()) {
        setError('Please enter a competition name.');
        return;
      }
      setStep(2);
    } else if (step === 2) {
      if (format === 'single_elimination' && ![4, 8, 16].includes(teamCount)) {
        setTeamCount(8);
      }
      setStep(3);
    }
  };

  const handlePrev = () => {
    setError(null);
    if (step > 1) {
      setStep((s) => (s - 1) as Step);
    } else {
      onCancel();
    }
  };

  const handleCreate = () => {
    setError(null);
    try {
      const comp = StorageService.createCompetition({
        name: name.trim(),
        category,
        format,
        totalTeams: teamCount,
        startDate,
        matchDurationMinutes: durationMinutes,
      });

      if (generateNow) {
        const teams = StorageService.getTeams(comp.id);
        if (teams.length < 2) {
          for (let i = 1; i <= teamCount; i++) {
            StorageService.registerTeam({
              competitionId: comp.id,
              name: `Team ${String.fromCharCode(64 + i)}`,
              captain: `Captain ${i}`,
              rosterCount: category === 'esports' ? 5 : 11,
            });
          }
        }
        StorageService.generateFixturesForCompetition(comp.id);
      }

      onCreated(comp, generateNow);
    } catch (err: unknown) {
      setError(err instanceof Error ? err.message : 'Failed to create competition');
    }
  };

  return (
    <div className="max-w-2xl mx-auto py-6 px-4">
      {/* Step Indicator */}
      <FadeIn delay={0} y={20}>
        <div className="mb-8">
          <div className="flex items-center justify-between text-xs font-mono uppercase tracking-wider text-[#D7E2EA]/50 mb-3">
            <span className={step >= 1 ? 'text-[#B600A8] font-bold' : ''}>1. Discipline</span>
            <span className={step >= 2 ? 'text-[#B600A8] font-bold' : ''}>2. Format & Teams</span>
            <span className={step >= 3 ? 'text-[#B600A8] font-bold' : ''}>3. Schedule</span>
          </div>
          <div className="h-1.5 bg-[#171a21] rounded-full overflow-hidden border border-[#D7E2EA]/10">
            <div
              className="h-full bg-gradient-to-r from-[#B600A8] to-[#7621B0] transition-all duration-300"
              style={{ width: `${(step / 3) * 100}%` }}
            />
          </div>
        </div>
      </FadeIn>

      {/* Card Content */}
      <FadeIn delay={0.05} y={20}>
        <div className="bg-[#12151a] border border-[#D7E2EA]/15 rounded-3xl p-6 sm:p-8 shadow-xl">
          {error && (
            <div className="mb-6 p-3 bg-rose-500/10 border border-rose-500/30 rounded-xl text-rose-400 text-xs font-mono">
              {error}
            </div>
          )}

          {/* STEP 1: Name + Discipline */}
          {step === 1 && (
            <div className="space-y-6">
              <div>
                <h2 className="text-xl font-bold uppercase tracking-tight text-white">Competition Info</h2>
                <p className="text-xs text-[#D7E2EA]/60 mt-1">
                  Name your competition and select the discipline.
                </p>
              </div>

              <div>
                <label className="block text-xs font-mono uppercase tracking-wider text-[#D7E2EA]/70 mb-2">
                  Competition Name
                </label>
                <input
                  type="text"
                  value={name}
                  onChange={(e) => setName(e.target.value)}
                  placeholder="e.g. GEHU Winter Cup 2026"
                  className="w-full px-4 py-3 bg-[#0C0C0C] border border-[#D7E2EA]/15 rounded-2xl text-white text-sm focus:outline-none focus:border-[#B600A8] transition-colors"
                  autoFocus
                />
              </div>

              <div>
                <label className="block text-xs font-mono uppercase tracking-wider text-[#D7E2EA]/70 mb-2">
                  Discipline
                </label>
                <div className="grid grid-cols-1 sm:grid-cols-2 gap-3">
                  <button
                    type="button"
                    onClick={() => setCategory('sports')}
                    className={`p-4 rounded-2xl border text-left transition-all flex items-start gap-3 cursor-pointer ${
                      category === 'sports'
                        ? 'bg-[#18011F] border-[#B600A8] text-white shadow-[0_0_15px_rgba(182,0,168,0.2)]'
                        : 'bg-[#171a21] border-[#D7E2EA]/15 text-[#D7E2EA] hover:border-[#D7E2EA]/30'
                    }`}
                  >
                    <div className={`p-2.5 rounded-xl ${category === 'sports' ? 'bg-[#B600A8]/20 text-[#B600A8]' : 'bg-[#0C0C0C] text-[#D7E2EA]/60'}`}>
                      <Trophy className="w-5 h-5" />
                    </div>
                    <div>
                      <div className="text-sm font-bold text-white">Sports</div>
                      <div className="text-xs text-[#D7E2EA]/60 mt-0.5">Football, Basketball, Cricket, etc.</div>
                    </div>
                  </button>

                  <button
                    type="button"
                    onClick={() => setCategory('esports')}
                    className={`p-4 rounded-2xl border text-left transition-all flex items-start gap-3 cursor-pointer ${
                      category === 'esports'
                        ? 'bg-[#18011F] border-[#B600A8] text-white shadow-[0_0_15px_rgba(182,0,168,0.2)]'
                        : 'bg-[#171a21] border-[#D7E2EA]/15 text-[#D7E2EA] hover:border-[#D7E2EA]/30'
                    }`}
                  >
                    <div className={`p-2.5 rounded-xl ${category === 'esports' ? 'bg-[#7621B0]/20 text-[#7621B0]' : 'bg-[#0C0C0C] text-[#D7E2EA]/60'}`}>
                      <Gamepad2 className="w-5 h-5" />
                    </div>
                    <div>
                      <div className="text-sm font-bold text-white">Esports</div>
                      <div className="text-xs text-[#D7E2EA]/60 mt-0.5">Valorant, CS2, Rocket League, etc.</div>
                    </div>
                  </button>
                </div>
              </div>
            </div>
          )}

          {/* STEP 2: Format & Teams */}
          {step === 2 && (
            <div className="space-y-6">
              <div>
                <h2 className="text-xl font-bold uppercase tracking-tight text-white">Tournament Format</h2>
                <p className="text-xs text-[#D7E2EA]/60 mt-1">
                  Choose the structure and total participating teams.
                </p>
              </div>

              <div>
                <label className="block text-xs font-mono uppercase tracking-wider text-[#D7E2EA]/70 mb-2">
                  Format
                </label>
                <div className="grid grid-cols-1 sm:grid-cols-2 gap-3">
                  <button
                    type="button"
                    onClick={() => {
                      setFormat('single_elimination');
                      if (teamCount === 6) setTeamCount(8);
                    }}
                    className={`p-4 rounded-2xl border text-left transition-all cursor-pointer ${
                      format === 'single_elimination'
                        ? 'bg-[#18011F] border-[#B600A8] text-white shadow-[0_0_15px_rgba(182,0,168,0.2)]'
                        : 'bg-[#171a21] border-[#D7E2EA]/15 text-[#D7E2EA] hover:border-[#D7E2EA]/30'
                    }`}
                  >
                    <div className="text-sm font-bold text-white">Single Elimination</div>
                    <div className="text-xs text-[#D7E2EA]/60 mt-1">
                      Classic knockout tree. Loser is eliminated; winner advances.
                    </div>
                  </button>

                  <button
                    type="button"
                    onClick={() => setFormat('round_robin')}
                    className={`p-4 rounded-2xl border text-left transition-all cursor-pointer ${
                      format === 'round_robin'
                        ? 'bg-[#18011F] border-[#B600A8] text-white shadow-[0_0_15px_rgba(182,0,168,0.2)]'
                        : 'bg-[#171a21] border-[#D7E2EA]/15 text-[#D7E2EA] hover:border-[#D7E2EA]/30'
                    }`}
                  >
                    <div className="text-sm font-bold text-white">Round Robin</div>
                    <div className="text-xs text-[#D7E2EA]/60 mt-1">
                      Every team plays every other team. Ranked on points table.
                    </div>
                  </button>
                </div>
              </div>

              <div>
                <label className="block text-xs font-mono uppercase tracking-wider text-[#D7E2EA]/70 mb-2">
                  Number of Teams
                </label>
                <div className="grid grid-cols-4 gap-2">
                  {[4, 6, 8, 16].map((count) => {
                    const isKnockoutInvalid = format === 'single_elimination' && count === 6;
                    return (
                      <button
                        key={count}
                        type="button"
                        disabled={isKnockoutInvalid}
                        onClick={() => setTeamCount(count)}
                        className={`py-3 rounded-2xl border text-sm font-mono transition-all tabular-nums cursor-pointer ${
                          teamCount === count
                            ? 'bg-[#B600A8] text-white border-[#B600A8] font-bold shadow-sm'
                            : isKnockoutInvalid
                            ? 'opacity-30 cursor-not-allowed border-[#D7E2EA]/10 bg-[#0C0C0C] text-[#D7E2EA]/40'
                            : 'bg-[#171a21] border-[#D7E2EA]/15 text-[#D7E2EA] hover:border-[#D7E2EA]/30'
                        }`}
                      >
                        {count} Teams
                      </button>
                    );
                  })}
                </div>
                {format === 'single_elimination' && (
                  <p className="text-[11px] text-[#D7E2EA]/50 mt-2 font-mono">
                    Single elimination brackets require 4, 8, or 16 teams.
                  </p>
                )}
              </div>
            </div>
          )}

          {/* STEP 3: Start date/time + Duration */}
          {step === 3 && (
            <div className="space-y-6">
              <div>
                <h2 className="text-xl font-bold uppercase tracking-tight text-white">Schedule & Setup</h2>
                <p className="text-xs text-[#D7E2EA]/60 mt-1">
                  Set timing details and generate tournament fixtures.
                </p>
              </div>

              <div className="grid grid-cols-1 sm:grid-cols-2 gap-4">
                <div>
                  <label className="block text-xs font-mono uppercase tracking-wider text-[#D7E2EA]/70 mb-2">
                    <Calendar className="w-3.5 h-3.5 inline mr-1 text-[#D7E2EA]/50" /> Start Date
                  </label>
                  <input
                    type="date"
                    value={startDate}
                    onChange={(e) => setStartDate(e.target.value)}
                    className="w-full px-4 py-2.5 bg-[#0C0C0C] border border-[#D7E2EA]/15 rounded-2xl text-white text-sm focus:outline-none focus:border-[#B600A8]"
                  />
                </div>

                <div>
                  <label className="block text-xs font-mono uppercase tracking-wider text-[#D7E2EA]/70 mb-2">
                    <Clock className="w-3.5 h-3.5 inline mr-1 text-[#D7E2EA]/50" /> Start Time
                  </label>
                  <input
                    type="text"
                    value={startTime}
                    onChange={(e) => setStartTime(e.target.value)}
                    placeholder="10:00 AM"
                    className="w-full px-4 py-2.5 bg-[#0C0C0C] border border-[#D7E2EA]/15 rounded-2xl text-white text-sm focus:outline-none focus:border-[#B600A8] font-mono"
                  />
                </div>
              </div>

              <div>
                <label className="block text-xs font-mono uppercase tracking-wider text-[#D7E2EA]/70 mb-2">
                  Match Duration (minutes)
                </label>
                <div className="grid grid-cols-3 gap-2">
                  {[30, 60, 90].map((dur) => (
                    <button
                      key={dur}
                      type="button"
                      onClick={() => setDurationMinutes(dur)}
                      className={`py-2.5 rounded-2xl border text-sm font-mono tabular-nums transition-all cursor-pointer ${
                        durationMinutes === dur
                          ? 'bg-[#B600A8] text-white border-[#B600A8] font-bold shadow-sm'
                          : 'bg-[#171a21] border-[#D7E2EA]/15 text-[#D7E2EA] hover:border-[#D7E2EA]/30'
                      }`}
                    >
                      {dur} min
                    </button>
                  ))}
                </div>
              </div>

              <div className="pt-3 border-t border-[#D7E2EA]/15">
                <label className="flex items-start gap-3 cursor-pointer select-none">
                  <input
                    type="checkbox"
                    checked={generateNow}
                    onChange={(e) => setGenerateNow(e.target.checked)}
                    className="mt-1 w-4 h-4 rounded bg-[#0C0C0C] border-[#D7E2EA]/30 text-[#B600A8] focus:ring-[#B600A8]"
                  />
                  <div>
                    <div className="text-sm font-bold text-white flex items-center gap-1.5">
                      <Sparkles className="w-3.5 h-3.5 text-[#B600A8]" />
                      Generate fixtures now
                    </div>
                    <div className="text-xs text-[#D7E2EA]/60 mt-0.5">
                      Automatically seeds placeholder teams and builds the full match schedule.
                    </div>
                  </div>
                </label>
              </div>
            </div>
          )}

          {/* Action Buttons */}
          <div className="mt-8 pt-5 border-t border-[#D7E2EA]/15 flex items-center justify-between">
            <button
              type="button"
              onClick={handlePrev}
              className="px-5 py-2.5 text-xs font-mono uppercase tracking-wider rounded-full border-2 border-[#D7E2EA] text-[#D7E2EA] hover:bg-[#D7E2EA]/10 transition-colors flex items-center gap-1.5 cursor-pointer"
            >
              <ArrowLeft className="w-3.5 h-3.5" />
              {step === 1 ? 'Cancel' : 'Back'}
            </button>

            {step < 3 ? (
              <button
                type="button"
                onClick={handleNext}
                style={{
                  background: 'linear-gradient(123deg, #18011F 7%, #B600A8 37%, #7621B0 72%, #BE4C00 100%)',
                  boxShadow: '0px 4px 4px rgba(181, 1, 167, 0.25), 4px 4px 12px #7721B1 inset',
                  outline: '2px solid #ffffff',
                  outlineOffset: '-3px',
                }}
                className="px-6 py-2.5 text-xs font-medium uppercase tracking-wider rounded-full text-white hover:opacity-90 transition-opacity flex items-center gap-1.5 cursor-pointer"
              >
                <span>Next Step</span>
                <ArrowRight className="w-3.5 h-3.5" />
              </button>
            ) : (
              <button
                type="button"
                onClick={handleCreate}
                style={{
                  background: 'linear-gradient(123deg, #18011F 7%, #B600A8 37%, #7621B0 72%, #BE4C00 100%)',
                  boxShadow: '0px 4px 4px rgba(181, 1, 167, 0.25), 4px 4px 12px #7721B1 inset',
                  outline: '2px solid #ffffff',
                  outlineOffset: '-3px',
                }}
                className="px-7 py-2.5 text-xs font-medium uppercase tracking-wider rounded-full text-white hover:opacity-90 transition-opacity flex items-center gap-1.5 cursor-pointer shadow-lg"
              >
                <Check className="w-4 h-4" />
                <span>Create Competition</span>
              </button>
            )}
          </div>
        </div>
      </FadeIn>
    </div>
  );
};
