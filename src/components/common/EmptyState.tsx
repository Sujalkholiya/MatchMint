import React from 'react';
import { LucideIcon, Inbox } from 'lucide-react';

interface EmptyStateProps {
  icon?: LucideIcon;
  title: string;
  description: string;
  action?: {
    label: string;
    onClick: () => void;
  };
  actionLabel?: string;
  onAction?: () => void;
}

export const EmptyState: React.FC<EmptyStateProps> = ({
  icon: Icon = Inbox,
  title,
  description,
  action,
  actionLabel,
  onAction,
}) => {
  const btnLabel = action?.label || actionLabel;
  const btnClick = action?.onClick || onAction;

  return (
    <div className="flex flex-col items-center justify-center py-12 px-4 text-center border border-dashed border-[#D7E2EA]/15 rounded-3xl bg-[#12151a]/40">
      <div className="w-12 h-12 rounded-2xl bg-[#171a21] border border-[#D7E2EA]/15 flex items-center justify-center text-[#D7E2EA]/60 mb-3 shadow-inner">
        <Icon className="w-6 h-6 text-[#D7E2EA]/60" />
      </div>
      <h3 className="text-base font-bold uppercase tracking-tight text-white">{title}</h3>
      <p className="text-xs text-[#D7E2EA]/60 max-w-sm mt-1 mb-5">{description}</p>
      {btnLabel && btnClick && (
        <button
          onClick={btnClick}
          style={{
            background: 'linear-gradient(123deg, #18011F 7%, #B600A8 37%, #7621B0 72%, #BE4C00 100%)',
            boxShadow: '0px 4px 4px rgba(181, 1, 167, 0.25), 4px 4px 12px #7721B1 inset',
            outline: '2px solid #ffffff',
            outlineOffset: '-3px',
          }}
          className="inline-flex items-center justify-center px-6 py-2.5 text-xs font-medium uppercase tracking-wider rounded-full text-white hover:opacity-90 transition-opacity cursor-pointer shadow-sm"
        >
          {btnLabel}
        </button>
      )}
    </div>
  );
};
