import React from 'react';
import { CompetitionCategory } from '../../types';
import { Trophy, Gamepad2 } from 'lucide-react';

interface CategoryBadgeProps {
  category: CompetitionCategory;
}

export const CategoryBadge: React.FC<CategoryBadgeProps> = ({ category }) => {
  return (
    <span className="inline-flex items-center gap-1.5 px-2.5 py-0.5 rounded-full text-xs font-mono uppercase tracking-wider bg-[#18011F] text-[#D7E2EA] border border-[#7621B0]/30">
      {category === 'sports' ? (
        <Trophy className="w-3 h-3 text-[#B600A8]" />
      ) : (
        <Gamepad2 className="w-3 h-3 text-[#7621B0]" />
      )}
      <span>{category === 'sports' ? 'Sports' : 'Esports'}</span>
    </span>
  );
};
