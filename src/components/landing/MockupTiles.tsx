import React, { useState } from 'react';
import { Trophy, Calendar, Users, CheckCircle2, Shield, ArrowRight, Swords } from 'lucide-react';

interface MockupTileProps {
  id: string;
  imageSrc: string;
  type: 'bracket' | 'standings' | 'match' | 'modal' | 'fixtures' | 'teams';
  title: string;
}

export const MockupTile: React.FC<MockupTileProps> = ({ id, imageSrc, type, title }) => {
  const [hasError, setHasError] = useState(false);

  return (
    <div className="w-[340px] sm:w-[420px] h-[220px] sm:h-[270px] shrink-0 rounded-2xl border border-[#D7E2EA]/15 bg-[#12151a] overflow-hidden shadow-xl relative select-none">
      {!hasError ? (
        <img
          src={imageSrc}
          alt={title}
          loading="lazy"
          onError={() => setHasError(true)}
          className="w-full h-full object-cover rounded-2xl"
        />
      ) : (
        /* Dynamic High-Fidelity UI Mockup */
        <div className="w-full h-full p-4 sm:p-5 flex flex-col justify-between bg-gradient-to-br from-[#12151a] via-[#171a21] to-[#1e232c] text-[#D7E2EA]">
          {/* Tile Header */}
          <div className="flex items-center justify-between pb-2.5 border-b border-[#D7E2EA]/10">
            <span className="font-mono text-[10px] sm:text-xs uppercase tracking-wider text-[#D7E2EA]/60">
              {title}
            </span>
            <span className="w-2 h-2 rounded-full bg-[#B600A8]" />
          </div>

          {/* Tile Center Body based on type */}
          {type === 'bracket' && (
            <div className="space-y-2 py-2">
              <div className="p-2.5 rounded-xl bg-[#0C0C0C]/80 border border-[#D7E2EA]/10 flex items-center justify-between">
                <span className="text-xs font-semibold text-white">Semi Final 01</span>
                <span className="font-mono text-xs text-[#B600A8] font-bold">2 — 0</span>
              </div>
              <div className="p-2.5 rounded-xl bg-gradient-to-r from-[#18011F] to-[#12151a] border border-[#B600A8]/30 flex items-center justify-between">
                <span className="text-xs font-semibold text-white">Championship Final</span>
                <span className="px-2 py-0.5 rounded bg-[#B600A8] text-white text-[10px] font-mono">
                  LIVE
                </span>
              </div>
            </div>
          )}

          {type === 'standings' && (
            <div className="space-y-1.5 py-1 text-xs">
              <div className="p-2 rounded-lg bg-[#0C0C0C]/80 border-l-2 border-l-[#B600A8] border-[#D7E2EA]/10 flex items-center justify-between">
                <div className="flex items-center gap-2">
                  <span className="font-mono text-[#D7E2EA]/50 font-bold">1</span>
                  <span className="font-semibold text-white">Falcons</span>
                </div>
                <span className="font-mono font-bold text-white tabular-nums">9 pts</span>
              </div>
              <div className="p-2 rounded-lg bg-[#0C0C0C]/40 border border-[#D7E2EA]/10 flex items-center justify-between opacity-80">
                <div className="flex items-center gap-2">
                  <span className="font-mono text-[#D7E2EA]/50 font-bold">2</span>
                  <span className="font-medium text-[#D7E2EA]">Mountain Strikers</span>
                </div>
                <span className="font-mono font-medium text-[#D7E2EA]/80 tabular-nums">6 pts</span>
              </div>
            </div>
          )}

          {type === 'match' && (
            <div className="p-3 rounded-xl bg-[#0C0C0C]/70 border border-[#D7E2EA]/10 space-y-2">
              <div className="flex items-center justify-between text-xs font-mono text-[#D7E2EA]/60">
                <span>Today • 4:30 PM</span>
                <span>Main Arena</span>
              </div>
              <div className="flex items-center justify-between font-bold text-white text-xs sm:text-sm">
                <span>Cyber Phantoms</span>
                <span className="text-[10px] font-mono font-normal px-2 py-0.5 rounded bg-[#1e232c] text-[#D7E2EA]/60">
                  VS
                </span>
                <span>Byte Raiders</span>
              </div>
            </div>
          )}

          {type === 'modal' && (
            <div className="p-3 rounded-xl bg-[#0C0C0C]/80 border border-[#B600A8]/30 space-y-1.5">
              <div className="flex items-center gap-1.5 text-xs font-semibold text-white">
                <CheckCircle2 className="w-3.5 h-3.5 text-[#B600A8]" />
                <span>Result Recorded</span>
              </div>
              <div className="text-[11px] font-mono text-[#D7E2EA]/60">
                • Standings auto-recalculated<br />
                • Winner slotted into Finals
              </div>
            </div>
          )}

          {type === 'fixtures' && (
            <div className="space-y-1.5 py-1">
              <div className="flex items-center justify-between text-[11px] font-mono p-2 rounded-lg bg-[#0C0C0C]/60 border border-[#D7E2EA]/10">
                <span className="text-white font-medium">Round Robin 03</span>
                <span className="text-[#D7E2EA]/60">Pitched Turf</span>
              </div>
              <div className="flex items-center justify-between text-[11px] font-mono p-2 rounded-lg bg-[#0C0C0C]/60 border border-[#D7E2EA]/10">
                <span className="text-white font-medium">Round Robin 04</span>
                <span className="text-[#D7E2EA]/60">Esports Lab A</span>
              </div>
            </div>
          )}

          {type === 'teams' && (
            <div className="p-3 rounded-xl bg-[#0C0C0C]/80 border border-[#D7E2EA]/10 space-y-2">
              <div className="flex items-center gap-2">
                <Shield className="w-4 h-4 text-[#B600A8]" />
                <span className="text-xs font-semibold text-white">Titan Esports</span>
              </div>
              <div className="flex items-center justify-between text-[10px] font-mono text-[#D7E2EA]/60">
                <span>Captain: Alex Vance</span>
                <span>5 Members</span>
              </div>
            </div>
          )}

          {/* Footer Bar */}
          <div className="pt-2 border-t border-[#D7E2EA]/10 flex items-center justify-between text-[10px] font-mono uppercase tracking-wider text-[#D7E2EA]/50">
            <span>MatchMint Engine</span>
            <span>Realtime View</span>
          </div>
        </div>
      )}
    </div>
  );
};

