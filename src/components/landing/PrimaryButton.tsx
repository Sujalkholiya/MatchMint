import React from 'react';

interface PrimaryButtonProps {
  label?: string;
  onClick?: () => void;
  className?: string;
  children?: React.ReactNode;
}

export const PrimaryButton: React.FC<PrimaryButtonProps> = ({
  label = 'Open App',
  onClick,
  className = '',
  children,
}) => {
  return (
    <button
      type="button"
      onClick={onClick}
      style={{
        background: 'linear-gradient(123deg, #18011F 7%, #B600A8 37%, #7621B0 72%, #BE4C00 100%)',
        boxShadow: '0px 4px 4px rgba(181, 1, 167, 0.25), 4px 4px 12px #7721B1 inset',
        outline: '2px solid #ffffff',
        outlineOffset: '-3px',
      }}
      className={`rounded-full text-white font-medium uppercase tracking-widest text-xs sm:text-sm md:text-base px-8 py-3 sm:px-10 sm:py-3.5 md:px-12 md:py-4 hover:opacity-90 transition-opacity duration-200 cursor-pointer focus-visible:ring-2 focus-visible:ring-white focus-visible:ring-offset-2 focus-visible:ring-offset-[#0C0C0C] select-none inline-flex items-center justify-center ${className}`}
    >
      {children || label}
    </button>
  );
};

