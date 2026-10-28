import React from 'react';
import { ChevronRight } from 'lucide-react';

interface TopbarProps {
  title: string;
  subtitle?: string;
  breadcrumb?: string;
  primaryAction?: {
    label: string;
    onClick: () => void;
  };
}

export const Topbar: React.FC<TopbarProps> = ({
  title,
  subtitle,
  breadcrumb,
  primaryAction,
}) => {
  return (
    <header className="h-14 bg-[#12151a] border-b border-[#D7E2EA]/15 px-6 flex items-center justify-between sticky top-0 z-20">
      {/* Title & Context */}
      <div className="flex flex-col justify-center">
        {breadcrumb ? (
          <div className="flex items-center gap-1.5 text-xs">
            <span className="text-[#D7E2EA]/60 font-mono">{breadcrumb}</span>
            <ChevronRight className="w-3.5 h-3.5 text-[#D7E2EA]/40" />
            <span className="font-semibold text-white">{title}</span>
          </div>
        ) : (
          <h1 className="text-sm font-semibold tracking-tight text-white">
            {title}
          </h1>
        )}
        {subtitle && (
          <p className="text-[11px] text-[#D7E2EA]/60 mt-0.5 tracking-tight">
            {subtitle}
          </p>
        )}
      </div>

      {/* Primary Action Button */}
      {primaryAction && (
        <button
          onClick={primaryAction.onClick}
          style={{
            background: 'linear-gradient(123deg, #18011F 7%, #B600A8 37%, #7621B0 72%, #BE4C00 100%)',
            boxShadow: '0px 4px 4px rgba(181, 1, 167, 0.25), 4px 4px 12px #7721B1 inset',
            outline: '2px solid #ffffff',
            outlineOffset: '-3px',
          }}
          className="inline-flex items-center gap-1.5 px-4 py-1.5 text-xs font-medium uppercase tracking-wider rounded-full text-white hover:opacity-90 transition-opacity cursor-pointer"
        >
          <span>{primaryAction.label}</span>
        </button>
      )}
    </header>
  );
};
