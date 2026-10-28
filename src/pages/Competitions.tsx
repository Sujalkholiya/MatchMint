import React, { useState } from 'react';
import { Competition, Match, Team } from '../types';
import { CompetitionCard } from '../components/competition/CompetitionCard';
import { EmptyState } from '../components/common/EmptyState';
import { Trophy } from 'lucide-react';
import { FadeIn } from '../components/landing/FadeIn';

interface CompetitionsProps {
  competitions: Competition[];
  teams: Team[];
  matches: Match[];
  onSelectCompetition: (comp: Competition) => void;
  onCreateCompetition: () => void;
}

export const Competitions: React.FC<CompetitionsProps> = ({
  competitions,
  teams,
  matches,
  onSelectCompetition,
  onCreateCompetition,
}) => {
  const [selectedCategory, setSelectedCategory] = useState<string>('all');
  const [selectedFormat, setSelectedFormat] = useState<string>('all');

  const filteredCompetitions = competitions.filter((comp) => {
    if (selectedCategory !== 'all' && comp.category !== selectedCategory) return false;
    if (selectedFormat !== 'all' && comp.format !== selectedFormat) return false;
    return true;
  });

  return (
    <div className="p-6 max-w-5xl mx-auto space-y-6">
      {/* Header */}
      <FadeIn delay={0} y={20}>
        <div className="flex flex-col sm:flex-row sm:items-center justify-between gap-4 pb-2">
          <div>
            <h1 className="hero-heading text-3xl md:text-5xl font-black uppercase tracking-tight">
              Competitions
            </h1>
            <p className="text-xs text-[#D7E2EA]/60 mt-1">
              Active sports and esports tournaments across campus.
            </p>
          </div>

          <button
            onClick={onCreateCompetition}
            style={{
              background: 'linear-gradient(123deg, #18011F 7%, #B600A8 37%, #7621B0 72%, #BE4C00 100%)',
              boxShadow: '0px 4px 4px rgba(181, 1, 167, 0.25), 4px 4px 12px #7721B1 inset',
              outline: '2px solid #ffffff',
              outlineOffset: '-3px',
            }}
            className="px-5 py-2 text-xs font-medium uppercase tracking-wider rounded-full text-white hover:opacity-90 transition-opacity inline-flex items-center gap-1.5 cursor-pointer self-start sm:self-auto"
          >
            <span>+ New Tournament</span>
          </button>
        </div>
      </FadeIn>

      {/* Category Pills & Format Filter */}
      <FadeIn delay={0.05} y={20}>
        <div className="flex flex-wrap items-center justify-between gap-3">
          <div className="flex items-center gap-2">
            {[
              { id: 'all', label: 'All Disciplines' },
              { id: 'sports', label: 'Sports' },
              { id: 'esports', label: 'Esports' },
            ].map((cat) => (
              <button
                key={cat.id}
                onClick={() => setSelectedCategory(cat.id)}
                className={`px-4 py-1.5 text-xs font-mono uppercase tracking-wider rounded-full transition-all cursor-pointer ${
                  selectedCategory === cat.id
                    ? 'bg-[#B600A8] text-white font-bold shadow-sm'
                    : 'bg-[#12151a] text-[#D7E2EA]/70 hover:text-white border border-[#D7E2EA]/15'
                }`}
              >
                {cat.label}
              </button>
            ))}
          </div>

          <select
            value={selectedFormat}
            onChange={(e) => setSelectedFormat(e.target.value)}
            className="text-xs font-mono py-1.5 px-3 rounded-full bg-[#12151a] border border-[#D7E2EA]/15 text-[#D7E2EA] focus:outline-none focus:border-[#B600A8]"
          >
            <option value="all">All Formats</option>
            <option value="single_elimination">Single Elimination</option>
            <option value="round_robin">Round Robin</option>
          </select>
        </div>
      </FadeIn>

      {/* Competitions Grid */}
      <FadeIn delay={0.1} y={20}>
        {filteredCompetitions.length > 0 ? (
          <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4">
            {filteredCompetitions.map((comp) => (
              <CompetitionCard
                key={comp.id}
                competition={comp}
                teams={teams}
                matches={matches}
                onSelect={onSelectCompetition}
              />
            ))}
          </div>
        ) : (
          <EmptyState
            icon={Trophy}
            title="No Tournaments Match Your Filter"
            description="Try switching categories or create a new tournament."
            action={{
              label: 'Create Tournament',
              onClick: onCreateCompetition,
            }}
          />
        )}
      </FadeIn>
    </div>
  );
};
