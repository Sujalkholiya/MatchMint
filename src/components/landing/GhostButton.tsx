import React from 'react';

interface GhostButtonProps {
  label?: string;
  onClick?: () => void;
  className?: string;
  children?: React.ReactNode;
}

export const GhostButton: React.FC<GhostButtonProps> = ({
  label,
  onClick,
  className = '',
  children,
}) => {
  return (
    <button
      type="button"
      onClick={onClick}
      className={`rounded-full border-2 border-[#D7E2EA] text-[#D7E2EA] font-medium uppercase tracking-widest text-sm sm:text-base px-8 py-3 sm:px-10 sm:py-3.5 hover:bg-[#D7E2EA]/10 transition-colors duration-200 cursor-pointer focus-visible:ring-2 focus-visible:ring-[#D7E2EA] focus-visible:ring-offset-2 focus-visible:ring-offset-[#0C0C0C] select-none inline-flex items-center justify-center ${className}`}
    >
      {children || label}
    </button>
  );
};

