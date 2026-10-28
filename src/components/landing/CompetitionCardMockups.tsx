import React, { useState } from 'react';
import { Trophy, Swords, Calendar, Users, Award, Shield } from 'lucide-react';

interface CompetitionMediaItemProps {
  slug: string;
  index: number;
  alt: string;
  heightClass?: string;
  type: 'bracket' | 'standings' | 'overview';
  compName: string;
  category: 'Sports' | 'Esports';
}

export const CompetitionMediaItem: React.FC<CompetitionMediaItemProps> = ({
  slug,
  index,
  alt,
  heightClass = 'h-full',
  type,
  compName,
  category,
}) => {
  const [hasError, setHasError] = useState(false);
  const src = `/projects/${slug}-${index}.png`;

  return (
    <div
      className={`w-full ${heightClass} rounded-2xl sm:rounded-3xl border border-[#D7E2EA]/15 bg-[#12151a] overflow-hidden relative select-none`}
    >
      {!hasError ? (
        <img
          src={src}
          alt={alt}
          loading="lazy"
          onError={() => setHasError(true)}
          className="w-full h-full object-cover rounded-2xl sm:rounded-3xl"
        />
      ) : (
        /* Fallback React Mockup graphic */
        <div className="w-full h-full p-4 sm:p-6 flex flex-col justify-between bg-gradient-to-br from-[#12151a] via-[#171a21] to-[#18011F] text-[#D7E2EA]">
          <div className="flex items-center justify-between pb-2 border-b border-[#D7E2EA]/10 text-xs font-mono">
            <span className="text-[#D7E2EA]/60 uppercase tracking-wider">{compName}</span>
            <span className="px-2 py-0.5 rounded bg-[#B600A8]/20 text-[#D7E2EA] text-[10px] uppercase font-bold">
              {category}
            </span>
          </div>

          {type === 'bracket' && (
            <div className="space-y-2.5 my-auto">
              <div className="p-3 rounded-xl bg-[#0C0C0C]/80 border border-[#D7E2EA]/10 flex items-center justify-between">
                <div>
                  <div className="text-xs font-bold text-white">Falcons [FAL]</div>
                  <div className="text-[10px] font-mono text-[#D7E2EA]/50">Seed 1</div>
                </div>
                <div className="font-mono text-base font-bold text-[#B600A8]">3</div>
              </div>
              <div className="p-3 rounded-xl bg-[#0C0C0C]/40 border border-[#D7E2EA]/10 flex items-center justify-between opacity-70">
                <div>
                  <div className="text-xs font-medium text-[#D7E2EA]">Mountain Strikers</div>
                  <div className="text-[10px] font-mono text-[#D7E2EA]/40">Seed 3</div>
                </div>
                <div className="font-mono text-base font-medium text-[#D7E2EA]/60">1</div>
              </div>
            </div>
          )}

          {type === 'standings' && (
            <div className="space-y-2 my-auto text-xs">
              <div className="p-2.5 rounded-lg bg-[#0C0C0C]/90 border-l-2 border-l-[#B600A8] border-[#D7E2EA]/10 flex items-center justify-between">
                <span className="font-semibold text-white">1. Apex Rangers</span>
                <span className="font-mono font-bold text-white">9 Pts</span>
              </div>
              <div className="p-2.5 rounded-lg bg-[#0C0C0C]/50 border border-[#D7E2EA]/10 flex items-center justify-between">
                <span className="text-[#D7E2EA]">2. Shadow Protocol</span>
                <span className="font-mono text-[#D7E2EA]/80">6 Pts</span>
              </div>
              <div className="p-2.5 rounded-lg bg-[#0C0C0C]/30 border border-[#D7E2EA]/10 flex items-center justify-between opacity-70">
                <span className="text-[#D7E2EA]/80">3. Iron Legion</span>
                <span className="font-mono text-[#D7E2EA]/60">3 Pts</span>
              </div>
            </div>
          )}

          {type === 'overview' && (
            <div className="space-y-3 my-auto">
              <div className="grid grid-cols-2 gap-2 text-center">
                <div className="p-3 rounded-xl bg-[#0C0C0C]/70 border border-[#D7E2EA]/10">
                  <div className="font-mono text-lg font-bold text-white">8</div>
                  <div className="text-[10px] font-mono uppercase text-[#D7E2EA]/50">Teams Seeded</div>
                </div>
                <div className="p-3 rounded-xl bg-[#0C0C0C]/70 border border-[#D7E2EA]/10">
                  <div className="font-mono text-lg font-bold text-[#B600A8]">7</div>
                  <div className="text-[10px] font-mono uppercase text-[#D7E2EA]/50">Knockout Matches</div>
                </div>
              </div>
              <div className="p-3 rounded-xl bg-[#0C0C0C]/80 border border-[#B600A8]/30 flex items-center justify-between text-xs">
                <span className="text-white font-medium">Automatic Tree Advancement</span>
                <Trophy className="w-4 h-4 text-[#B600A8]" />
              </div>
            </div>
          )}

          <div className="pt-2 border-t border-[#D7E2EA]/10 flex items-center justify-between text-[10px] font-mono uppercase tracking-wider text-[#D7E2EA]/50">
            <span>Tournament Record</span>
            <span>MatchMint</span>
          </div>
        </div>
      )}
    </div>
  );
};

