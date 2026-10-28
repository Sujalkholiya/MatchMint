import React, { useState } from 'react';
import { Trophy, Swords, ShieldCheck, Sparkles } from 'lucide-react';

interface HeroVisualProps {
  className?: string;
}

export const HeroVisual: React.FC<HeroVisualProps> = ({ className = '' }) => {
  const [imageError, setImageError] = useState(false);

  return (
    <div className={`relative select-none pointer-events-auto ${className}`}>
      {!imageError ? (
        <img
          src="/images/hero.png"
          alt="MatchMint Tournament Championship Bracket"
          onError={() => setImageError(true)}
          className="w-full h-auto object-contain drop-shadow-[0_20px_40px_rgba(182,0,168,0.4)]"
        />
      ) : (
        /* Styled React placeholder bracket card if hero.png is not in /public */
        <div className="relative w-full rounded-[32px] sm:rounded-[40px] border-2 border-[#D7E2EA]/30 bg-gradient-to-br from-[#18011F] via-[#12151a] to-[#171a21] p-5 sm:p-7 shadow-[0_20px_60px_rgba(182,0,168,0.35)] backdrop-blur-xl overflow-hidden">
          {/* Subtle Ambient Radial Glow */}
          <div className="absolute -top-12 -right-12 w-48 h-48 rounded-full bg-[#B600A8]/20 blur-3xl pointer-events-none" />
          <div className="absolute -bottom-12 -left-12 w-48 h-48 rounded-full bg-[#7621B0]/25 blur-3xl pointer-events-none" />

          {/* Header */}
          <div className="flex items-center justify-between pb-3.5 border-b border-[#D7E2EA]/15">
            <div className="flex items-center gap-2">
              <span className="w-2.5 h-2.5 rounded-full bg-[#B600A8] animate-pulse" />
              <span className="text-[11px] sm:text-xs font-mono uppercase tracking-wider text-[#D7E2EA]/70">
                Grand Championship
              </span>
            </div>
            <div className="flex items-center gap-1.5 px-2.5 py-0.5 rounded-full bg-[#B600A8]/20 border border-[#B600A8]/40 text-[#D7E2EA] text-[10px] sm:text-[11px] font-mono">
              <Sparkles className="w-3 h-3 text-[#B600A8]" />
              <span>Live Slot</span>
            </div>
          </div>

          {/* Match Teams & Score */}
          <div className="my-4 space-y-2.5">
            {/* Team 1 */}
            <div className="flex items-center justify-between p-3 rounded-2xl bg-[#0C0C0C]/70 border border-[#D7E2EA]/15">
              <div className="flex items-center gap-2.5">
                <div className="w-8 h-8 rounded-xl bg-gradient-to-br from-[#B600A8] to-[#7621B0] flex items-center justify-center text-white font-bold text-xs shadow-sm">
                  F
                </div>
                <div>
                  <div className="text-xs sm:text-sm font-semibold text-white">Falcons</div>
                  <div className="text-[10px] font-mono text-[#D7E2EA]/60">Seed #1 • Sports</div>
                </div>
              </div>
              <div className="font-mono text-base sm:text-xl font-black text-white px-2.5 py-0.5 rounded-lg bg-[#18011F] border border-[#B600A8]/40 tabular-nums">
                3
              </div>
            </div>

            {/* Team 2 */}
            <div className="flex items-center justify-between p-3 rounded-2xl bg-[#0C0C0C]/50 border border-[#D7E2EA]/10 opacity-75">
              <div className="flex items-center gap-2.5">
                <div className="w-8 h-8 rounded-xl bg-[#1e232c] flex items-center justify-center text-[#D7E2EA]/80 font-bold text-xs">
                  M
                </div>
                <div>
                  <div className="text-xs sm:text-sm font-medium text-[#D7E2EA]">Mountain Strikers</div>
                  <div className="text-[10px] font-mono text-[#D7E2EA]/50">Seed #3 • Sports</div>
                </div>
              </div>
              <div className="font-mono text-base sm:text-xl font-bold text-[#D7E2EA]/70 px-2.5 py-0.5 rounded-lg bg-[#0C0C0C] border border-[#D7E2EA]/15 tabular-nums">
                1
              </div>
            </div>
          </div>

          {/* Winner Footer Banner */}
          <div className="pt-3 border-t border-[#D7E2EA]/15 flex items-center justify-between">
            <div className="flex items-center gap-2 text-xs font-semibold text-white">
              <Trophy className="w-4 h-4 text-[#B600A8]" />
              <span>Falcons Advanced</span>
            </div>
            <span className="text-[10px] font-mono uppercase tracking-wider text-[#D7E2EA]/60">
              Final Result
            </span>
          </div>
        </div>
      )}
    </div>
  );
};

