import React, { useState, useEffect } from 'react';
import { Match, Team, Competition } from '../../types';
import { Modal } from '../common/Modal';
import { CheckCircle2, Trophy } from 'lucide-react';

interface EnterResultModalProps {
  isOpen: boolean;
  onClose: () => void;
  match: Match | null;
  teams: Team[];
  competition: Competition | null;
  onSubmit: (matchId: string, team1Score: number, team2Score: number, tieBreakerDetail?: string) => void;
}

export const EnterResultModal: React.FC<EnterResultModalProps> = ({
  isOpen,
  onClose,
  match,
  teams,
  competition,
  onSubmit,
}) => {
  const [score1, setScore1] = useState<string>('');
  const [score2, setScore2] = useState<string>('');
  const [tieBreaker, setTieBreaker] = useState<string>('');
  const [tieWinnerId, setTieWinnerId] = useState<string>('');
  const [error, setError] = useState<string | null>(null);
  const [isSavedSuccess, setIsSavedSuccess] = useState(false);

  useEffect(() => {
    if (match) {
      setScore1(match.team1Score !== null ? String(match.team1Score) : '');
      setScore2(match.team2Score !== null ? String(match.team2Score) : '');
      setTieBreaker(match.tieBreakerDetails || '');
      setTieWinnerId(match.winnerId || '');
      setError(null);
      setIsSavedSuccess(false);
    }
  }, [match]);

  if (!match) return null;

  const team1 = teams.find((t) => t.id === match.team1Id);
  const team2 = teams.find((t) => t.id === match.team2Id);

  const num1 = parseInt(score1, 10);
  const num2 = parseInt(score2, 10);
  const isDraw = !isNaN(num1) && !isNaN(num2) && num1 === num2;
  const isElimination = competition?.format === 'single_elimination';

  let calculatedWinner: Team | null = null;
  if (!isNaN(num1) && !isNaN(num2)) {
    if (num1 > num2 && team1) calculatedWinner = team1;
    else if (num2 > num1 && team2) calculatedWinner = team2;
    else if (isDraw && isElimination && tieWinnerId) {
      calculatedWinner = tieWinnerId === team1?.id ? team1 : team2 || null;
    }
  }

  const handleSubmit = (e: React.FormEvent) => {
    e.preventDefault();
    if (isNaN(num1) || isNaN(num2)) {
      setError('Please enter valid numerical scores for both teams.');
      return;
    }
    if (num1 < 0 || num2 < 0) {
      setError('Scores cannot be negative.');
      return;
    }
    if (isDraw && isElimination && !calculatedWinner) {
      setError('Knockout tournament matches cannot conclude as a draw. Please select a tie-break winner.');
      return;
    }

    onSubmit(match.id, num1, num2, tieBreaker.trim() || undefined);
    setIsSavedSuccess(true);
  };

  const handleFinish = () => {
    setIsSavedSuccess(false);
    onClose();
  };

  return (
    <Modal
      isOpen={isOpen}
      onClose={isSavedSuccess ? handleFinish : onClose}
      title="Enter Match Result"
      subtitle={`${match.roundName} • ${match.venue || 'Main Arena'}`}
      maxWidth="md"
    >
      {isSavedSuccess ? (
        <div className="py-4 space-y-5">
          <div className="p-4 rounded-2xl bg-[#0C0C0C] border border-[#B600A8]/40 space-y-3">
            <div className="flex items-center gap-2 text-sm font-semibold text-white">
              <CheckCircle2 className="w-5 h-5 shrink-0 text-[#B600A8]" />
              <span>Result recorded</span>
            </div>

            <div className="space-y-1.5 pl-7 text-xs text-[#D7E2EA]/70 font-mono">
              <div className="flex items-center gap-2">
                <span className="w-1.5 h-1.5 rounded-full bg-[#B600A8]" />
                <span>Leaderboard updated</span>
              </div>
              <div className="flex items-center gap-2">
                <span className="w-1.5 h-1.5 rounded-full bg-[#B600A8]" />
                <span>Next round updated</span>
              </div>
            </div>
          </div>

          <div className="flex justify-end pt-2">
            <button
              type="button"
              onClick={handleFinish}
              style={{
                background: 'linear-gradient(123deg, #18011F 7%, #B600A8 37%, #7621B0 72%, #BE4C00 100%)',
                boxShadow: '0px 4px 4px rgba(181, 1, 167, 0.25), 4px 4px 12px #7721B1 inset',
                outline: '2px solid #ffffff',
                outlineOffset: '-3px',
              }}
              className="px-6 py-2 text-xs font-medium uppercase tracking-wider rounded-full text-white hover:opacity-90 transition-opacity cursor-pointer"
            >
              Done
            </button>
          </div>
        </div>
      ) : (
        <form onSubmit={handleSubmit} className="space-y-5">
          {error && (
            <div className="p-3 rounded-2xl bg-rose-500/10 border border-rose-500/30 text-xs text-rose-300 font-mono">
              {error}
            </div>
          )}

          {/* Focused Score Inputs Grid */}
          <div className="grid grid-cols-2 gap-4 items-center">
            {/* Team 1 */}
            <div className="p-4 rounded-2xl bg-[#0C0C0C] border border-[#D7E2EA]/15 text-center space-y-2">
              <div className="text-xs font-mono text-[#D7E2EA]/50">
                #{team1?.seed || '1'} Home
              </div>
              <div className="font-bold text-white text-sm truncate">
                {team1?.name || 'Falcons'}
              </div>
              <div className="pt-1">
                <input
                  type="number"
                  min="0"
                  autoFocus
                  value={score1}
                  onChange={(e) => setScore1(e.target.value)}
                  placeholder="0"
                  className="w-20 mx-auto text-center font-mono text-2xl font-bold py-1.5 px-2 rounded-xl bg-[#12151a] border border-[#D7E2EA]/20 text-white focus:outline-none focus:border-[#B600A8]"
                />
              </div>
            </div>

            {/* Team 2 */}
            <div className="p-4 rounded-2xl bg-[#0C0C0C] border border-[#D7E2EA]/15 text-center space-y-2">
              <div className="text-xs font-mono text-[#D7E2EA]/50">
                #{team2?.seed || '3'} Away
              </div>
              <div className="font-bold text-white text-sm truncate">
                {team2?.name || 'Mountain Strikers'}
              </div>
              <div className="pt-1">
                <input
                  type="number"
                  min="0"
                  value={score2}
                  onChange={(e) => setScore2(e.target.value)}
                  placeholder="0"
                  className="w-20 mx-auto text-center font-mono text-2xl font-bold py-1.5 px-2 rounded-xl bg-[#12151a] border border-[#D7E2EA]/20 text-white focus:outline-none focus:border-[#B600A8]"
                />
              </div>
            </div>
          </div>

          {/* Winner Section */}
          <div className="p-3.5 rounded-2xl bg-[#0C0C0C] border border-[#D7E2EA]/15 text-xs flex items-center justify-between">
            <span className="text-[#D7E2EA]/60 font-mono">Winner:</span>
            {calculatedWinner ? (
              <span className="font-bold text-white flex items-center gap-1.5 font-mono">
                <Trophy className="w-3.5 h-3.5 text-[#B600A8]" />
                {calculatedWinner.name}
              </span>
            ) : (
              <span className="text-[#D7E2EA]/50 italic">
                Automatically determine winner.
              </span>
            )}
          </div>

          {/* Tie-breaker options if draw in elimination */}
          {isDraw && isElimination && (
            <div className="p-3.5 rounded-2xl bg-[#0C0C0C] border border-[#B600A8]/30 space-y-2 text-xs">
              <label className="block font-medium text-white">
                Elimination Decider / Tie-Break Winner:
              </label>
              <div className="grid grid-cols-2 gap-2">
                <button
                  type="button"
                  onClick={() => setTieWinnerId(team1?.id || '')}
                  className={`py-2 px-2 text-xs rounded-xl border transition-colors cursor-pointer ${
                    tieWinnerId === team1?.id
                      ? 'bg-[#18011F] border-[#B600A8] text-white font-bold'
                      : 'bg-[#12151a] border-[#D7E2EA]/15 text-[#D7E2EA]/60 hover:text-white'
                  }`}
                >
                  {team1?.name} Won Decider
                </button>
                <button
                  type="button"
                  onClick={() => setTieWinnerId(team2?.id || '')}
                  className={`py-2 px-2 text-xs rounded-xl border transition-colors cursor-pointer ${
                    tieWinnerId === team2?.id
                      ? 'bg-[#18011F] border-[#B600A8] text-white font-bold'
                      : 'bg-[#12151a] border-[#D7E2EA]/15 text-[#D7E2EA]/60 hover:text-white'
                  }`}
                >
                  {team2?.name} Won Decider
                </button>
              </div>
              <input
                type="text"
                value={tieBreaker}
                onChange={(e) => setTieBreaker(e.target.value)}
                placeholder="e.g. 5-4 on penalty kicks / overtime"
                className="w-full text-xs p-2.5 rounded-xl bg-[#12151a] border border-[#D7E2EA]/15 text-white placeholder:text-[#D7E2EA]/30 focus:outline-none focus:border-[#B600A8]"
              />
            </div>
          )}

          {/* Modal Actions */}
          <div className="flex items-center justify-end gap-2.5 pt-3 border-t border-[#D7E2EA]/15">
            <button
              type="button"
              onClick={onClose}
              className="px-4 py-2 text-xs font-mono uppercase tracking-wider rounded-full border-2 border-[#D7E2EA] text-[#D7E2EA] hover:bg-[#D7E2EA]/10 transition-colors cursor-pointer"
            >
              Cancel
            </button>
            <button
              type="submit"
              style={{
                background: 'linear-gradient(123deg, #18011F 7%, #B600A8 37%, #7621B0 72%, #BE4C00 100%)',
                boxShadow: '0px 4px 4px rgba(181, 1, 167, 0.25), 4px 4px 12px #7721B1 inset',
                outline: '2px solid #ffffff',
                outlineOffset: '-3px',
              }}
              className="px-6 py-2 text-xs font-medium uppercase tracking-wider rounded-full text-white hover:opacity-90 transition-opacity cursor-pointer"
            >
              Save Result
            </button>
          </div>
        </form>
      )}
    </Modal>
  );
};
